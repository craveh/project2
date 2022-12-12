#include <stdio.h>


typedef struct packet {
  int seq_no;
  int size;
  // int packet_type; //0-ack, 1-data, 2-FIN
  int offset;
  int is_last_packet;
  int is_data; // 1 if data, 0 if ack
  char data[256 + 1];
} Packet;

typedef struct packet_info {
  Packet packet;
  int send_time;
  int ack_recv; // only used in Selective Repeat
} PacketInfo;


int main(int argc, char const *argv[])

{
    PacketInfo packet_buffer[] = {};
    int i;
    for(i=0; i<5; i++){
        printf("TEST %d\n", packet_buffer[i].send_time);
        // packet_buffer[i] = packet_i_info;
    }
    // printf("%s", argv[1]);
    return 0;
}
