#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include "packet.h"


void receive_message_no_packet(int sockfd, struct sockaddr_in client_addr, int client_length){
    int n;
    char buffer[MAX_LENGTH + 1];
    char * filename = "received_file.txt";
    FILE * fp = fp = fopen(filename, "w");


    bzero(buffer, MAX_LENGTH);

    while(1){
        if (recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &client_length) < 0){
        printf("Couldn't receive\n");
        exit(1);
        }
        printf("Received message from IP: %s and port: %i\n",
            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        
        printf("Msg from client: %s\n", buffer);
        
        if(strcmp(buffer, "FIN") == 0){
            break;
        }
        // printf("Received: %s\n", buffer);
        fprintf(fp, "%s", buffer);
        bzero(buffer, MAX_LENGTH);
    }
    strcpy(buffer, "everything went well");
    
    if (sendto(sockfd, buffer, strlen(buffer), 0,
         (struct sockaddr*)&client_addr, client_length) < 0){
        perror("Can't send\n");
        exit(1);
    }

    fclose(fp);

}

void stop_and_wait(int sockfd, struct sockaddr_in client_addr, int client_length){
    int n, seq_no, is_eof;
    char buffer[MAX_LENGTH + 1];
    char * filename = "received_file.txt";
    FILE * fp = fp = fopen(filename, "w");
    Packet data_packet, ack_packet;
    is_eof = 0;
    seq_no = -1;


    bzero(buffer, MAX_LENGTH);

    while(!is_eof){
        if (recvfrom(sockfd, &data_packet, sizeof(data_packet), 0, (struct sockaddr*)&client_addr, &client_length) < 0){
            error("Couldn't receive packet\n");
        }
        if(data_packet.seq_no == (seq_no + 1)%2 && data_packet.is_data == 1){
            printf("[recv data] %d (%d) ACCEPTED\n", data_packet.offset, data_packet.size);
            fprintf(fp, "%s", data_packet.data);
            is_eof = data_packet.is_last_packet;
            seq_no =data_packet.seq_no;
            
        }else if (data_packet.seq_no != (seq_no + 1)%2){
            printf("[recv data] %d (%d) IGNORED\n", data_packet.offset, data_packet.size);
        }
        if (data_packet.is_data != 1){
            continue;
        }
        

        //create ack packet
        ack_packet.seq_no = seq_no;
        ack_packet.is_data = 0;
        ack_packet.is_last_packet = data_packet.is_last_packet;


        if (sendto(sockfd, &ack_packet, sizeof(ack_packet), 0, (struct sockaddr*)&client_addr, client_length) < 0){
            error("Can't send ack packet\n");
        }

        
    }
    

    fclose(fp);

}


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



    // receive_message_no_packet(sockfd, client_addr, client_length);
    stop_and_wait(sockfd, client_addr, client_length);

    
    close(sockfd);
    printf("[completed]\n");
    
    return 0;
}
