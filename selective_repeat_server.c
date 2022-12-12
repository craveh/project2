#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include "packet.h"

int main(int argc, char *argv[]){
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    int client_length = sizeof(client_addr);

    //check args
    if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
    }

    //create socket:
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    if(sockfd < 0){
        printf("Error while creating socket\n");
        return -1;
    }
    printf("Socket created successfully\n");
    
    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    // Bind to the set port and IP:
    if(bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Couldn't bind to the port\n");
        return -1;
    }
    printf("Done with binding\n");
    
    printf("Listening for incoming messages...\n\n");

    // struct timeval timeout;
    // timeout.tv_usec = DELAY;
    // if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0){
    //     error("setsockopt failed\n");
    // }


    int n, seq_no, is_eof, next_seq_no, offset, send_ack, print;
    // char buffer[MAX_LENGTH + 1];
    char * filename = "received_file_other.txt";
    FILE * fp = fp = fopen(filename, "w");
    Packet data_packet, ack_packet;
    Packet packet_i = {0,0,0,0,0,""};
    is_eof = next_seq_no = offset = send_ack = print = 0;
    seq_no = -1;
    PacketInfo temp;
    PacketInfo packet_i_info = {packet_i, 0, 0};
    PacketInfo packet_buffer[WINDOW_SIZE] = {};
    int i;

    //initialize packet_buffer
    for(i=0; i<WINDOW_SIZE; i++){
        packet_buffer[i] = packet_i_info;
    }


    // bzero(buffer, MAX_LENGTH);

    while(!is_eof){
        if (recvfrom(sockfd, &data_packet, sizeof(data_packet), 0, (struct sockaddr*)&client_addr, &client_length) < 0){
            error("Couldn't receive packet\n");
        }

        printf("Data seq_no %d, next seq: %d\n", data_packet.seq_no, next_seq_no);

        if(data_packet.seq_no == next_seq_no){
            memcpy(&temp.packet, &data_packet, sizeof(Packet));
            temp.ack_recv = 1; //send ack
            packet_buffer[next_seq_no] = temp;

            printf("[recv data] %d (%d) ACCEPTED\n", data_packet.offset, data_packet.size);
            fprintf(fp, "%s", data_packet.data);
            is_eof = data_packet.is_last_packet;
            seq_no =data_packet.seq_no;
            // send_ack = 0;
            //create ack packet
            ack_packet.seq_no = seq_no;
            ack_packet.is_data = 0;
            ack_packet.is_last_packet = data_packet.is_last_packet;
            if (sendto(sockfd, &ack_packet, sizeof(ack_packet), 0, (struct sockaddr*)&client_addr, client_length) < 0){
                error("Can't send ack packet\n");
            }

            next_seq_no = (next_seq_no +1 )%WINDOW_SIZE;
            //move window
            while(packet_buffer[next_seq_no].ack_recv){
                packet_buffer[next_seq_no].ack_recv = 0;
                next_seq_no = (next_seq_no +1 )%WINDOW_SIZE;
    
            }
            // printf("Next seq_no: %d\n", next_seq_no);

        }else if(packet_buffer[data_packet.seq_no].packet.offset < data_packet.offset){
            memcpy(&temp.packet, &data_packet, sizeof(Packet));
            temp.ack_recv = 1; //send ack
            packet_buffer[i] = temp; 
                
            ack_packet.seq_no = seq_no;
            ack_packet.is_data = 0;
            ack_packet.is_last_packet = data_packet.is_last_packet;
            if (sendto(sockfd, &ack_packet, sizeof(ack_packet), 0, (struct sockaddr*)&client_addr, client_length) < 0){
                error("Can't send ack packet\n");
            }
            printf("[recv data] %d (%d) ACCEPTED\n", data_packet.offset, data_packet.size);

            // for(i=0; i<WINDOW_SIZE; i++){
            //     printf("In for loop\n");
            //     packet_i_info = packet_buffer[i];
            //     if(i == data_packet.seq_no && packet_i_info.packet.offset <= data_packet.offset ){
            //         printf("In if in for\n");
            //         memcpy(&temp.packet, &data_packet, sizeof(Packet));
            //         temp.ack_recv = 1; //send ack
            //         packet_buffer[i] = temp; 
                
            //         ack_packet.seq_no = seq_no;
            //         ack_packet.is_data = 0;
            //         ack_packet.is_last_packet = data_packet.is_last_packet;
            //         if (sendto(sockfd, &ack_packet, sizeof(ack_packet), 0, (struct sockaddr*)&client_addr, client_length) < 0){
            //             error("Can't send ack packet\n");
            //         }
            //         printf("[recv data] %d (%d) ACCEPTED\n", data_packet.offset, data_packet.size);


            //     }else{
            //         printf("[recv data] %d (%d) IGNORED\n", data_packet.offset, data_packet.size);
            //     }
            // }
        }else{
            printf("[recv data] %d (%d) IGNORED\n", data_packet.offset, data_packet.size);
        }

        
        

        

        
    }
    

    fclose(fp);

    
    close(sockfd);
    printf("[completed]\n");
    
    return 0;
}





// #include <stdio.h>
// #include <string.h>
// #include <sys/socket.h>
// #include <arpa/inet.h>
// #include <unistd.h>
// #include <stdlib.h>
// #include "packet.h"

// int main(int argc, char *argv[]){
//     int sockfd;
//     struct sockaddr_in server_addr, client_addr;
//     int client_length = sizeof(client_addr);

//     //check args
//     if (argc < 2) {
//          fprintf(stderr,"ERROR, no port provided\n");
//          exit(1);
//     }

//     //create socket:
//     sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
//     if(sockfd < 0){
//         printf("Error while creating socket\n");
//         return -1;
//     }
//     printf("Socket created successfully\n");
    
//     // Set port and IP:
//     server_addr.sin_family = AF_INET;
//     server_addr.sin_port = htons(atoi(argv[1]));
//     server_addr.sin_addr.s_addr = INADDR_ANY;
    
//     // Bind to the set port and IP:
//     if(bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
//         printf("Couldn't bind to the port\n");
//         return -1;
//     }
//     printf("Done with binding\n");
    
//     printf("Listening for incoming messages...\n\n");


//     int n, seq_no, is_eof;
//     char buffer[MAX_LENGTH + 1];
//     char * filename = "received_file.txt";
//     FILE * fp = fp = fopen(filename, "w");
//     Packet data_packet, ack_packet;
//     is_eof = 0;
//     seq_no = -1;


//     bzero(buffer, MAX_LENGTH);

//     while(!is_eof){
//         if (recvfrom(sockfd, &data_packet, sizeof(data_packet), 0, (struct sockaddr*)&client_addr, &client_length) < 0){
//             error("Couldn't receive packet\n");
//         }
//         if(data_packet.seq_no == (seq_no + 1)%WINDOW_SIZE && data_packet.is_data == 1){
//             printf("[recv data] %d (%d) ACCEPTED\n", data_packet.offset, data_packet.size);
//             fprintf(fp, "%s", data_packet.data);
//             is_eof = data_packet.is_last_packet;
//             seq_no =data_packet.seq_no;
            
//         }else if (data_packet.seq_no != (seq_no + 1)%WINDOW_SIZE){
//             printf("[recv data] %d (%d) IGNORED\n", data_packet.offset, data_packet.size);
//         }
//         if (data_packet.is_data != 1){
//             continue;
//         }
        

//         //create ack packet
//         ack_packet.seq_no = seq_no;
//         ack_packet.is_data = 0;
//         ack_packet.is_last_packet = data_packet.is_last_packet;


//         if (sendto(sockfd, &ack_packet, sizeof(ack_packet), 0, (struct sockaddr*)&client_addr, client_length) < 0){
//             error("Can't send ack packet\n");
//         }

        
//     }
    

//     fclose(fp);

    
//     close(sockfd);
//     printf("[completed]\n");
    
//     return 0;
// }
