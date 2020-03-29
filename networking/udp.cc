#include "ip.h"

void UDP::send_packet(uint8_t *dest_ip, uint16_t src_port, uint16_t dest_port, void *data, int len){
    uint32_t totalLen = sizeof(UdpPacket) + len;
    UdpPacket *packet = (UdpPacket*)malloc(totalLen);
    packet->src_port = __builtin_bswap16(src_port);
    packet->dest_port = __builtin_bswap16(dest_port);
    packet->length = __builtin_bswap16(totalLen);
    packet->checksum = 0;

    //put the data in the packet
    memcpy((uint8_t*)packet + sizeof(UdpPacket), data, len);

    IP::send_packet(dest_ip, packet, totalLen);
}

void UDP::receive_packet(UdpPacket *packet){
    //fix data order
    packet->src_port = __builtin_bswap16(packet->src_port);
    packet->dest_port = __builtin_bswap16(packet->dest_port);
    packet->length = __builtin_bswap16(packet->length);
    packet->checksum = __builtin_bswap16(packet->checksum);

    uint8_t *data = (uint8_t*)packet + sizeof(UdpPacket);
    uint32_t datalen = packet->length - sizeof(UdpPacket);
    
    Debug::printf("| UDP incoming! \n");
    Debug::printf("| port %d -> %d\n", packet->src_port, packet->dest_port);
    Debug::printf("| data: ");
    for (uint32_t i = 0; i < datalen; i++){
        Debug::printf("0x%x:", data[i]);
    }
    Debug::printf("\n");

    //check socket table
    //find INET socket for port number and send to it
    //drop the packet otherwise
}
