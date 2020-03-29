#ifndef UDP_H
#define UDP_H

#include "../debug.h"
#include "../heap/gheith/heap.h"

struct UdpPacket {
    uint16_t src_port;
    uint16_t dest_port;
    uint16_t length;
    uint16_t checksum;
 } __attribute__((packed));

typedef struct UdpPacket UdpPacket;

class UDP {
public:
    static void send_packet(uint8_t *dest_ip, uint16_t src_port, uint16_t dest_port, void *data, int len);
    static void receive_packet(UdpPacket *packet);
};

#endif
