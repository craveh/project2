#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_LENGTH 1024

void read_data_no_packets(FILE* fp, int sockfd, struct sockaddr_in client_addr){
    // char * filename = "received_file.txt";
    // FILE * fp = fp = fopen(filename, "w");

    int n;
    char buffer[MAX_LENGTH];
    socklen_t addr_size;

    bzero(buffer, MAX_LENGTH);

    while(1){
        addr_size = sizeof(client_addr);
        // n = recvfrom(sockfd, buffer, MAX_LENGTH, 0, (struct sockaddr*)&client_addr, &addr_size);
        n = read(sockfd, buffer, MAX_LENGTH);
        if(n == -1){
            perror("Error reading:");
            exit(1);
        }
        if(strcmp(buffer, "FIN") == 0){
            break;
        }
        printf("Received: %s\n", buffer);
        fprintf(fp, "%s", buffer);
        bzero(buffer, MAX_LENGTH);
    }
    // fclose(fp);

}

int main(int argc, char *argv[])
{
    int sockfd, n;
    struct sockaddr_in server_addr, client_addr;
    int client_struct_length = sizeof(client_addr);
    FILE * fp;
    char * filename = "received_file.txt";


    //check args
    if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
    }
    
    // Create UDP socket:
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    if(sockfd < 0){
        perror("Error while creating socket\n");
        exit(1);
    }
    printf("Socket created successfully\n");
    
    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    // Bind to the set port and IP:
    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
        perror("ERROR on binding");
        exit(1);

    }
    

    fp = fopen(filename, "w");
    //receive data
    read_data_no_packets(fp, sockfd,client_addr);
    fclose(fp);

    //send file received to client
    char buffer[MAX_LENGTH];
    strcpy(buffer, "Received and wrote file");
    // if (sendto(sockfd, buffer, MAX_LENGTH, 0,
    //      (struct sockaddr*)&client_addr, client_struct_length) < 0){
    //     perror("Can't send\n");
    //     exit(1);
    // }
    n = write(sockfd, buffer, strlen(buffer));
    if (n == -1){
        perror("Error occured while sending data\n");
        exit(1);
    }

    // printf("Done with binding\n");
    
    // printf("Listening for incoming messages...\n\n");
    
    // // Receive client's message:
    // if (recvfrom(sockfd, client_message, sizeof(client_message), 0,
    //      (struct sockaddr*)&client_addr, &client_struct_length) < 0){
    //     printf("Couldn't receive\n");
    //     return -1;
    // }
    // printf("Received message from IP: %s and port: %i\n",
    //        inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    
    // printf("Msg from client: %s\n", client_message);
    
    // Respond to client:
    // strcpy(server_message, client_message);
    
    // if (sendto(sockfd, server_message, strlen(server_message), 0,
    //      (struct sockaddr*)&client_addr, client_struct_length) < 0){
    //     printf("Can't send\n");
    //     return -1;
    // }
    
    // Close the socket:
    close(sockfd);
    
    return 0;
}
