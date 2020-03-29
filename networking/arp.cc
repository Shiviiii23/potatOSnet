// #include <stdio.h>
#include "arp.h"
#include "../drivers/network_driver.h"

Atomic<uint32_t> ARP::numRepliesReceived {0};
ArpEntry ARP::mappings[ARP_MAX_NUM_MAPPINGS];
uint32_t ARP::map_index;

void ARP::receive(ArpPacket *packet) {
    
    //ARP spec says you should add the mapping before sending the opCode
    add_mapping((uint8_t*)&packet->senderProtocolAddress, (uint8_t*)&packet->senderHardwareAddress);

    switch (__builtin_bswap16(packet->opCode)) {
        case ARP_REQUEST: { 
            Debug::printf("| Someone sent an ARP request, is it for me?");

            // Is this request for me? if so, modify the packet into a reply and send it back
            if (K::bytecmp(packet->targetProtocolAddress, IP::ip, 4)) {
                Debug::printf(" for me.\n");
                
                //the sender to us (what is inside the packet) is now our target
                memcpy(&packet->targetProtocolAddress, &packet->senderProtocolAddress, 4);
                memcpy(&packet->targetHardwareAddress, &packet->senderHardwareAddress, 6);

                //finish the reply
                memcpy(&packet->senderProtocolAddress, IP::ip, 4);
                NIC::get_mac_addr(packet->senderHardwareAddress);

                packet->opCode = __builtin_bswap16(ARP_REPLY);

                packet->protocolAddressLength = 4;
                packet->hardwareAddressLength = 6;

                packet->hardwareType = __builtin_bswap16(ARP_HARDWARE_ETHERNET);
                packet->protocolType = __builtin_bswap16(EthernetProtocol::IP);

                Debug::printf("| sent reply back\n");
                Ethernet::send_frame(
                    (uint8_t*)&packet->targetHardwareAddress,
                    (uint8_t*)packet,
                    sizeof(ArpPacket),
                    EthernetProtocol::ARP);
            } else {
                Debug::printf(" Not for me.\n");
            }

            break;
        }
        case ARP_REPLY: { 
            Debug::printf("| Someone sent me an ARP reply\n");

            add_mapping((uint8_t*)&packet->senderProtocolAddress, (uint8_t*)&packet->senderHardwareAddress);
            break;
        }
        default:
            Debug::printf("| ARP opcode didn't match %d\n", packet->opCode);
            break;
    }
}

void ARP::send_request(uint8_t* destination_protocol) {
    ArpPacket* packet = (ArpPacket*)malloc(sizeof(ArpPacket)); 

    memcpy(&packet->senderProtocolAddress, IP::ip, 4);
    NIC::get_mac_addr(packet->senderHardwareAddress);

    // Set destination IP address. MAC address isn't needed for requests (since we don't know it!)
    memcpy(&packet->targetProtocolAddress, destination_protocol, 4);

    // Set opcode
    packet->opCode = __builtin_bswap16(ARP_REQUEST);

    // Set lengths
    packet->hardwareAddressLength = 6;
    packet->protocolAddressLength = 4;

    // Set hardware type
    packet->hardwareType = __builtin_bswap16(ARP_HARDWARE_ETHERNET);

    // Set protocol = IPv4
    packet->protocolType = __builtin_bswap16(EthernetProtocol::IP);

    // Now send it with ethernet
    Ethernet::send_frame((uint8_t*)Ethernet::broadcast_mac, (uint8_t*)packet, sizeof(ArpPacket), EthernetProtocol::ARP);
}

void ARP::add_mapping(uint8_t *ip, uint8_t *mac) {
    for(uint32_t i = 0; i < ARP_MAX_NUM_MAPPINGS; i++) {
        if(K::bytecmp((uint8_t*)&mappings[i].ip, ip, 4)) {
            memcpy(&mappings[i].mac, mac, 6);
            print_mappings();
            return;
        }
    }
    // Stores the IP mapping in the table
    memcpy(&mappings[map_index].ip, ip, 4);
    memcpy(&mappings[map_index].mac, mac, 6);

    map_index++;
    numRepliesReceived.add_fetch(1);

    if(map_index >= ARP_MAX_NUM_MAPPINGS)
        // If overflow - wrap around (shouldn't happen in our case because our network will be small and perfect)
        map_index = 0;

    print_mappings();
}

uint8_t* ARP::lookup(uint8_t *ip) {
    for(uint32_t i = 0; i < ARP_MAX_NUM_MAPPINGS; i++) {
        if(K::bytecmp((uint8_t*)&mappings[i].ip, ip, 4)) return (uint8_t*)&mappings[i].mac;
    }
    auto was = numRepliesReceived.get();
    send_request(ip);
    while(numRepliesReceived.get() <= was) yield(); //sorry Dr. gheith this is bad
    for(uint32_t i = 0; i < ARP_MAX_NUM_MAPPINGS; i++) {
        if(K::bytecmp((uint8_t*)&mappings[i].ip, ip, 4)) return (uint8_t*)&mappings[i].mac;
    }
    Debug::printf("| lookup try again\n");
    return lookup(ip);
}

void ARP::print_mappings() {
    for(uint32_t i = 0; i < ARP_MAX_NUM_MAPPINGS; i++) {
        uint8_t* ip = (uint8_t*)&mappings[i].ip;
        uint8_t* mac = (uint8_t*)&mappings[i].mac;
        Debug::printf("| %d.%d.%d.%d -> %02x.%02x.%02x.%02x.%02x.%02x\n",
            ip[0], ip[1], ip[2], ip[3],
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]
        );
    }
}
