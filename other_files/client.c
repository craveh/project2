#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define SIZE 1024

void transmit_data_no_packets(FILE* fp, int sockfd, struct sockaddr_in dest_addr){
    int n;
    char buffer[SIZE];
    
    bzero(buffer, SIZE);
    while (fgets(buffer, SIZE, fp) != NULL){
        printf("Sending: %s\n", buffer);
        // n = sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
        n = write(sockfd, buffer, strlen(buffer));
        if (n == -1){
            perror("Error occured while sending data\n");
            exit(1);
        }
        bzero(buffer, SIZE);
    }
    //send signal that total file was transmitted
    strcpy(buffer, "FIN");
    write(sockfd,buffer,strlen(buffer));
    // n = sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    
    
}

int main(int argc, char *argv[])
{
    int sockfd, n;
    struct sockaddr_in server_addr;
    // char server_message[2000], client_message[2000];
    int server_struct_length = sizeof(server_addr);
    // char buffer[SIZE];
    char server_msg[SIZE];
    FILE * fp;

    //check args
    if (argc < 4) {
       fprintf(stderr,"usage %s hostname port filename\n", argv[0]);
       exit(0);
    }
    
    
    //create socket and set server_addr info
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    printf("TEST\n");
    
    if(sockfd < 0){
        perror("Error while creating socket\n");
        exit(1);
    }
    printf("Socket created successfully\n");
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    // inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    if (connect(sockfd,(struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
        perror("ERROR connecting");
        exit(1);
    }


    //open file
    fp = fopen(argv[3], "r");
    if (fp == NULL){
        printf("Error reading file %s", argv[3]);
        exit(1);
    }
    
    //transmit file contents
    transmit_data_no_packets(fp, sockfd, server_addr);
    fclose(fp);
   
    //receive message from server
    bzero(server_msg, SIZE);

    if(recvfrom(sockfd, server_msg, SIZE, 0,
         (struct sockaddr*)&server_addr, &server_struct_length) < 0){
        printf("Error while receiving server's msg\n");
        return -1;
    }					
    // n = read(sockfd, server_msg, SIZE);
    // if (n < 0){
    //     perror("ERROR reading from socket");
    // }
    printf("Server says: %s\n", server_msg);

    
    // Close the socket:
    close(sockfd);
    
    return 0;
}

