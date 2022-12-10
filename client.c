#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "packet.h"

//change using AWS, now change local

void send_data(char * filename, int sockfd, struct sockaddr_in server_addr, int server_length){
    int n;
    char buffer[MAX_LENGTH + 1];
    
    bzero(buffer, MAX_LENGTH);
    FILE * fp;
    fp = fopen(filename, "r");
    int read_result;

    // while (fgets(buffer, MAX_LENGTH, fp) != NULL){
    //     printf("Sending: %s\n", buffer);
    //     if(sendto(sockfd, buffer, strlen(buffer), 0,
    //      (struct sockaddr*)&server_addr, server_length) < 0){
    //     printf("Unable to send message\n");
    //     exit(1);
    //     }
    // }
    while(1){
        read_result = fread(buffer, 1, MAX_LENGTH, fp);
        buffer[read_result] = '\0';
        printf("FREAD %d: %s\n\n", read_result, buffer);

        if(read_result > 0){
            // printf("Sending: %s\n", buffer);
            if(sendto(sockfd, buffer, strlen(buffer), 0,
            (struct sockaddr*)&server_addr, server_length) < 0){
            printf("Unable to send message\n");
            exit(1);
            }
            if(feof(fp)){
                printf("END OF FILE\n");
                break;  
            }
        }else{
            printf("Error Reading\n");
            exit(1);
        }

    }
        
        bzero(buffer, MAX_LENGTH);
    
    //send signal that total file was transmitted
    strcpy(buffer, "FIN");
    if(sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&server_addr, server_length) < 0){
        printf("Unable to send message\n");
        exit(1);
    }

    bzero(buffer, MAX_LENGTH);

    if(recvfrom(sockfd, buffer, sizeof(buffer), 0,
         (struct sockaddr*)&server_addr, &server_length) < 0){
        printf("Error while receiving server's msg\n");
        exit(1);
    }
    
    printf("Server's response: %s\n", buffer);

}

void stop_and_wait(char * filename, int sockfd, struct sockaddr_in server_addr, int server_length){
    int n, is_eof, seq_no, can_send, resend, offset,done;
    char buffer[MAX_LENGTH + 1];
    // bzero(buffer, MAX_LENGTH);
    FILE * fp;
    fp = fopen(filename, "r");
    int read_result;
    Packet send_packet, recv_packet;
    clock_t start_prog, end_prog, sent_time;
    double runtime;


    is_eof = seq_no = resend = offset = done = 0;
    can_send = 1;

    start_prog = clock();
    while(!done){
        if (can_send){
            bzero(buffer, MAX_LENGTH);

            read_result = fread(buffer, 1, MAX_LENGTH, fp);
            if(ferror(fp)){
                error("Error Reading File");
            }
            if(read_result == 0){
                strcpy(buffer, "\n");
            }
            buffer[read_result] = '\0';

            //build packet
            send_packet.offset = offset;
            offset += read_result; //update offset
            send_packet.seq_no = seq_no;
            send_packet.size = read_result;
            send_packet.is_data = 1;
            strcpy(send_packet.data, buffer);
            send_packet.is_last_packet = feof(fp);

            // if(feof(fp)){
            //     is_eof = 1;
            //     send_packet.is_last_packet = 1;
            // } else{
            //     send_packet.is_last_packet = 0;
            // }

            can_send = 0;
            //send packet
             if(sendto(sockfd, &send_packet, sizeof(send_packet), 0, (struct sockaddr*)&server_addr, server_length) < 0){
                error("Unable to send message\n");
            }
            sent_time = clock();
            printf("[send data] %d (%d)\n", send_packet.offset, send_packet.size);
            // resend = 0;
        }

        //check if timeout
        if (((double) clock()- sent_time) / CLOCKS_PER_SEC >TIMEOUT){
            //resend
            if(sendto(sockfd, &send_packet, sizeof(send_packet), 0, (struct sockaddr*)&server_addr, server_length) < 0){
                error("Unable to send message\n");
            }
            sent_time = clock(); //restart clock
            printf("[resend data] %d (%d)\n", send_packet.offset, send_packet.size);
        }

        // struct timeval timeout;
        // timeout
        // // timeout.tv_usec = 210000;
        // if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0){
        //     error("setsockopt failed\n");
        // }

        //get ack
        if(recvfrom(sockfd, &recv_packet, sizeof(recv_packet), 0, (struct sockaddr*)&server_addr, &server_length) < 0){
            error("Error while receiving server's msg\n");
        }
        if (recv_packet.seq_no == seq_no && recv_packet.is_data == 0){
            printf("[recv ack] %d\n", recv_packet.seq_no);
            seq_no = (seq_no + 1) % 2;
            can_send = 1;
            if(feof(fp)){
                done = 1;
            }
        }
    }
    end_prog = clock();
    runtime = ((double)(end_prog - start_prog)) / CLOCKS_PER_SEC;
    printf("Sent %d bytes in %f seconds.\n", offset, runtime);
    printf("Thoughput: %f bytes per second\n", offset/runtime);
    printf("[completed]\n");
}

int main(int argc, char *argv[]){
    int sockfd;
    struct sockaddr_in server_addr;
    int server_length = sizeof(server_addr);
    struct timeval timeout;


    //check args
    if (argc < 4) {
       fprintf(stderr,"usage %s hostname port filename\n", argv[0]);
       exit(0);
    }

    
    // Create socket:
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    if(sockfd < 0){
        printf("Error while creating socket\n");
        // return -1;
        exit(1);
    }
    printf("Socket created successfully\n");
    
    // Set port and IP:
    server_addr.sin_family = AF_INET;
    // server_addr.sin_port = htons(2000);
    server_addr.sin_port = htons(atoi(argv[2]));
    // server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    //set socket recv time timeout
    // timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 500000;
    if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0){
        error("setsockopt failed\n");
    }

    
    // send_data(argv[3], sockfd, server_addr, server_length);
    stop_and_wait(argv[3], sockfd, server_addr, server_length);

    close(sockfd);
    
    return 0;
}

