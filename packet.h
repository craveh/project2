#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

#ifndef PACKET_H
#define PACKET_H
#define MAX_LENGTH 128

typedef struct packet{
    int seq_no;
    int size;
    // int packet_type; //0-ack, 1-data, 2-FIN
    int is_last_packet;
    int is_data; //1 if data, 0 if ack
    char data[MAX_LENGTH + 1];
} Packet;

void error(char * error_message){
    perror(error_message);
    exit(1);
}



#endif