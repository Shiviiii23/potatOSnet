#ifndef ARP_H
#define ARP_H

#include "../debug.h"
#include "../std/stdint.h"
#include "../std/libk.h"
#include "ethernet.h"
#include "ip.h"

#define ARP_HARDWARE_ETHERNET 1
#define ARP_MAX_NUM_MAPPINGS 10
#define ARP_REQUEST 1
#define ARP_REPLY 2

// ARP packet header
struct ArpPacket {
    uint16_t hardwareType;
    uint16_t protocolType;
    uint8_t hardwareAddressLength;
    uint8_t protocolAddressLength;
    uint16_t opCode;

    uint8_t senderHardwareAddress[6];
    uint8_t senderProtocolAddress[4];
    uint8_t targetHardwareAddress[6];
    uint8_t targetProtocolAddress[4];

};

typedef struct ArpPacket ArpPacket;

// Mapping entry
struct ArpEntry {
    uint32_t ip;
    uint64_t mac;
};

typedef struct ArpEntry ArpEntry;

class ARP {
private:
    static Atomic<uint32_t> numRepliesReceived;
    static ArpEntry mappings[ARP_MAX_NUM_MAPPINGS];
    static uint32_t map_index;
    static void add_mapping(uint8_t *ip, uint8_t *mac);
public:
    static void receive(ArpPacket *packet);
    static void send_request(uint8_t *destination_protocol);
    static uint8_t* lookup(uint8_t *ip);
    static void print_mappings();
};

#endif
