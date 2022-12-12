// #include <stdio.h>
// #include <string.h>
// #include <sys/socket.h>
// #include <arpa/inet.h>
// #include <unistd.h>
// #include <stdlib.h>
// #include <time.h>
// #include "packet.h"

// int main(int argc, char *argv[]){
//     int sockfd;
//     struct sockaddr_in server_addr;
//     int server_length = sizeof(server_addr);

//     //check args
//     if (argc < 4) {
//        fprintf(stderr,"usage %s hostname port filename\n", argv[0]);
//        exit(0);
//     }

    
//     // Create socket:
//     sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
//     if(sockfd < 0){
//         printf("Error while creating socket\n");
//         // return -1;
//         exit(1);
//     }
//     printf("Socket created successfully\n");
    
//     // Set port and IP:
//     server_addr.sin_family = AF_INET;
//     server_addr.sin_port = htons(atoi(argv[2]));
//     server_addr.sin_addr.s_addr = inet_addr(argv[1]);

//     //set socket recv time timeout
//     struct timeval timeout;
//     timeout.tv_usec = DELAY;
//     if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0){
//         error("setsockopt failed\n");
//     }

    
//     // send_data(argv[3], sockfd, server_addr, server_length);
//     // stop_and_wait(argv[3], sockfd, server_addr, server_length);
//     // go_back_n(argv[3], sockfd, server_addr, server_length);


//     int n, buffer_head, seq_no, buffer_full, offset, done, next_recv_index;
//     char buffer[MAX_LENGTH + 1];
//     // bzero(buffer, MAX_LENGTH);
//     FILE * fp = fopen(argv[3], "r");
//     int read_result;
//     Packet send_packet, recv_packet, packet_i;
//     PacketInfo temp, packet_i_info;
//     PacketInfo packet_buffer[WINDOW_SIZE];
//     clock_t start_prog, end_prog, sent_time;
//     double runtime;

//     buffer_head = seq_no = buffer_full = offset = done = next_recv_index = 0;

//     start_prog = clock();
//     while(!done){

//         while(!buffer_full && !feof(fp)){
//             bzero(buffer, MAX_LENGTH);

//             read_result = fread(buffer, 1, MAX_LENGTH, fp);
//             if(ferror(fp)){
//                 error("Error Reading File");
//             }
//             if(read_result == 0){
//                 strcpy(buffer, "\n");
//             }
//             buffer[read_result] = '\0';

//             //build packet
//             send_packet.offset = offset;
//             offset += read_result; //update offset
//             send_packet.seq_no = (seq_no++) % WINDOW_SIZE; //sets and updates seq_no
//             send_packet.size = read_result;
//             send_packet.is_data = 1;
//             strcpy(send_packet.data, buffer);
//             send_packet.is_last_packet = feof(fp);

//             //send packet
//             if(sendto(sockfd, &send_packet, sizeof(send_packet), 0, (struct sockaddr*)&server_addr, server_length) < 0){
//                 error("Unable to send message\n");
//             }
//             sent_time = clock();

//             //save info into buffer
//             memcpy(&temp.packet, &send_packet, sizeof(Packet));
//             temp.send_time = sent_time;
//             temp.ack_recv = 0;
//             packet_buffer[buffer_head]  = temp;
//             buffer_head = (buffer_head + 1) % WINDOW_SIZE;
//             if(buffer_head == next_recv_index){
//                 buffer_full = 1;
//             }
//             printf("[send data] %d (%d)\n", send_packet.offset, send_packet.size);
//         }

//         packet_i_info = packet_buffer[next_recv_index];
//         // printf("TIME: %f\n", ((double) clock()- sent_time) / CLOCKS_PER_SEC);
//         if (((double) clock()- packet_i_info.send_time) / CLOCKS_PER_SEC > TIMEOUT){
//             //timeout so resend window
//             for(int i=0; i<WINDOW_SIZE; i++){
//                 if(i==buffer_head && i != 0){
//                     continue;
//                 }
                
//                 packet_i_info = packet_buffer[(next_recv_index + 1)%WINDOW_SIZE];
//                 if(packet_i_info.ack_recv){
//                     continue;
//                 }
//                 packet_i = packet_i_info.packet;


//                 //resend
//                 if(sendto(sockfd, &packet_i, sizeof(packet_i), 0, (struct sockaddr*)&server_addr, server_length) < 0){
//                     error("Unable to send message\n");
//                 }
//                 packet_i_info.send_time = clock(); //restart clock
//                 printf("[resend data] %d (%d)\n", packet_i.offset, packet_i.size);

//             }
            
//         }
//         bzero(buffer, MAX_LENGTH);

//         if (buffer_full || feof(fp)){

//             if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0){
//                 error("setsockopt failed\n");
//             }

//             //get ack
//             n=1;
//             while(n){
//                 printf("TEST N\n");
//                 n = recvfrom(sockfd, &recv_packet, sizeof(recv_packet), 0, (struct sockaddr*)&server_addr, &server_length);
            
//             if(n>0){
//                 printf("recv packet offset: %d\n", recv_packet.offset);
//                 for(int i=0; i<WINDOW_SIZE; i++){
//                     if(packet_i_info.ack_recv){
//                         printf("TEST IN IF\n");
//                         continue;
//                     }
//                     packet_i_info = packet_buffer[next_recv_index];
//                     packet_i = packet_i_info.packet;
//                     if(recv_packet.seq_no == packet_i.seq_no && recv_packet.is_data == 0 && recv_packet.offset == packet_i.offset){
//                         packet_i_info.ack_recv = 1;
//                         printf("[recv ack] %d\n", recv_packet.seq_no);
//                         if(next_recv_index == recv_packet.seq_no){
//                             next_recv_index = (next_recv_index + 1) % WINDOW_SIZE;
//                             buffer_full = 0;
//                         }
//                         if(feof(fp) && next_recv_index == buffer_head){
//                         done = 1;
//                     }

//                     }

                
//                 }
                
//             }

//             }
          

//         }  
//     }
//     end_prog = clock();
//     runtime = ((double)(end_prog - start_prog)) / CLOCKS_PER_SEC;
//     printf("Sent %d bytes in %f seconds.\n", offset, runtime);
//     printf("Thoughput: %f bytes per second\n", offset/runtime);
//     printf("[completed]\n");



//     close(sockfd);
    
//     return 0;
// }

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "packet.h"

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
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    //set socket recv time timeout
    timeout.tv_usec = DELAY;
    if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0){
        error("setsockopt failed\n");
    }

    
    // send_data(argv[3], sockfd, server_addr, server_length);
    // stop_and_wait(argv[3], sockfd, server_addr, server_length);
    // go_back_n(argv[3], sockfd, server_addr, server_length);


    int n, buffer_head, seq_no, buffer_full, offset, done, next_recv_index;
    char buffer[MAX_LENGTH + 1];
    // bzero(buffer, MAX_LENGTH);
    FILE * fp = fopen(argv[3], "r");
    int read_result;
    Packet send_packet, recv_packet, packet_i;
    PacketInfo temp, packet_i_info;
    PacketInfo packet_buffer[WINDOW_SIZE];
    clock_t start_prog, end_prog, sent_time;
    double runtime;

    buffer_head = seq_no = buffer_full = offset = done = next_recv_index = 0;

    start_prog = clock();
    while(!done){

        while(!buffer_full && !feof(fp)){
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
            send_packet.seq_no = (seq_no++) % WINDOW_SIZE; //sets and updates seq_no
            send_packet.size = read_result;
            send_packet.is_data = 1;
            strcpy(send_packet.data, buffer);
            send_packet.is_last_packet = feof(fp);

            //send packet
            if(sendto(sockfd, &send_packet, sizeof(send_packet), 0, (struct sockaddr*)&server_addr, server_length) < 0){
                error("Unable to send message\n");
            }
            sent_time = clock();

            //save info into buffer
            memcpy(&temp.packet, &send_packet, sizeof(Packet));
            temp.send_time = sent_time;
            temp.ack_recv = 0;
            packet_buffer[buffer_head]  = temp;
            buffer_head = (buffer_head + 1) % WINDOW_SIZE;
            if(buffer_head == next_recv_index){
                buffer_full = 1;
            }
            printf("[send data] %d (%d)\n", send_packet.offset, send_packet.size);
        }

        packet_i_info = packet_buffer[next_recv_index];
        // printf("TIME: %f\n", ((double) clock()- sent_time) / CLOCKS_PER_SEC);
        if (((double) clock()- packet_i_info.send_time) / CLOCKS_PER_SEC > TIMEOUT){
            //timeout so resend window
            for(int i=0; i<WINDOW_SIZE; i++){
                if(i==buffer_head && i != 0){
                    continue;
                }
                packet_i_info = packet_buffer[(next_recv_index + 1)%WINDOW_SIZE];
                packet_i = packet_i_info.packet;
                if(packet_i_info.ack_recv == 1){
                    continue;
                }

                //resend
                if(sendto(sockfd, &packet_i, sizeof(packet_i), 0, (struct sockaddr*)&server_addr, server_length) < 0){
                    error("Unable to send message\n");
                }
                packet_i_info.send_time = clock(); //restart clock
                printf("[resend data] %d (%d)\n", packet_i.offset, packet_i.size);

            }
            
        }
        bzero(buffer, MAX_LENGTH);

        if (buffer_full || feof(fp)){

            if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0){
                error("setsockopt failed\n");
            }

            //get ack
            n = recvfrom(sockfd, &recv_packet, sizeof(recv_packet), 0, (struct sockaddr*)&server_addr, &server_length);
            
            if(n>0){
                packet_i_info = packet_buffer[next_recv_index];
                packet_i = packet_i_info.packet;
                if (recv_packet.seq_no == packet_i.seq_no && recv_packet.is_data == 0){
                    printf("[recv ack] %d\n", recv_packet.seq_no);
                    next_recv_index = (next_recv_index + 1) % WINDOW_SIZE;
                    buffer_full = 0;

                    if(feof(fp) && next_recv_index == buffer_head){
                        done = 1;
                    }

                }else if(recv_packet.offset == packet_buffer[recv_packet.seq_no].packet.offset){
                    printf("[recv ack] %d\n", recv_packet.seq_no);
                    packet_buffer[recv_packet.seq_no].ack_recv = 1;

                }
                
                if(packet_buffer[next_recv_index].ack_recv == 1){
                    next_recv_index = (next_recv_index + 1) % WINDOW_SIZE;
                    buffer_full = 0;
                }
            }

        }  
    }
    end_prog = clock();
    runtime = ((double)(end_prog - start_prog)) / CLOCKS_PER_SEC;
    printf("Sent %d bytes in %f seconds.\n", offset, runtime);
    printf("Thoughput: %f bytes per second\n", offset/runtime);
    printf("[completed]\n");



    close(sockfd);
    
    return 0;
}



