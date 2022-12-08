#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include "packet.h"

//change using AWS

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
    int n, is_eof, seq_no;
    char buffer[MAX_LENGTH + 1];
    struct timeval timeout;
    bzero(buffer, MAX_LENGTH);
    FILE * fp;
    fp = fopen(filename, "r");
    int read_result;
    Packet send_packet, recv_packet;
    is_eof = 0;
    seq_no = 1;

    while(1){
        read_result = fread(buffer, 1, MAX_LENGTH, fp);
        buffer[read_result] = '\0';
        // printf("FREAD %d: %s\n\n", read_result, buffer);
        if(seq_no == 1){
            seq_no = 0;
        }else{
            seq_no = 1;
        }

        if(read_result >= 0){
            //handle read_result is 0
            if (ferror(fp)){
                error("Error reading file\n");
            }
            if(read_result == 0){
                strcpy(buffer, "\n");
            }
            //build packet
            send_packet.seq_no = seq_no;
            send_packet.size = strlen(buffer);
            send_packet.is_data = 1;
            strcpy(send_packet.data, buffer);
            if(feof(fp)){
                is_eof = 1;
                send_packet.is_last_packet = 1;
            } else{
                send_packet.is_last_packet = 0;
            }

            // printf("Sending: %s\n", buffer);

            //send packet
            if(sendto(sockfd, &send_packet, sizeof(send_packet), 0, (struct sockaddr*)&server_addr, server_length) < 0){
                error("Unable to send message\n");
            }
            printf("[send data] %s (%d)\n", buffer, read_result);

            //wait for ack
            if(recvfrom(sockfd, &recv_packet, sizeof(recv_packet), 0, (struct sockaddr*)&server_addr, &server_length) < 0){
                error("Error while receiving server's msg\n");
            }
            printf("[recv ack] %d\n", recv_packet.seq_no);

            if(is_eof){
                break;
            }
            // seq_no++; 
        }
        // if(read_result <MAX_LENGTH){
        //     if(feof(fp)){
        //         printf("READ RESULT %d\n", read_result);
        //         break;
        //     }
        //     if (ferror(fp)){
        //         error("Error reading file\n");

        //     }
        // }

    }
        
     


}

int main(int argc, char *argv[]){
    int sockfd;
    struct sockaddr_in server_addr;
    int server_length = sizeof(server_addr);

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
    
    
    // send_data(argv[3], sockfd, server_addr, server_length);
    stop_and_wait(argv[3], sockfd, server_addr, server_length);

    close(sockfd);
    
    return 0;
}

