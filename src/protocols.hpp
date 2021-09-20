
#pragma once


#define FIN 0x01
#define SYN 0x02
#define RST 0x04
#define PUSH 0x08
#define ACK 0x10
#define URG 0x20
#define ECE 0x40
#define CWR 0x80

struct ipv4
{
    uint8_t ver : 4;
    uint8_t header_len : 4;
    uint8_t _not_used;
    uint16_t length;
    uint16_t id;
    uint16_t flags : 3;
    uint16_t frag_offset : 13;
    uint8_t time_to_live;
    uint8_t protocol;
    uint16_t checksum;
    uint32_t source;
    uint32_t destination;

};


struct ip_and_ports
{
    struct ipv4 ip;
    
    uint16_t src_port;
    uint16_t dst_port;
};


struct tcp
{
    struct ipv4 ip;
    
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t seqnum;
    uint32_t acknum;
    uint8_t offset;
    uint8_t flag;
};

std::string ipToString(uint32_t ip)
{
    char buffer[16];
    
    char* ip_b = (char*) (&ip);

    // true if big endian
    snprintf(buffer, 16, "%hhu.%hhu.%hhu.%hhu", ip_b[0], ip_b[1], ip_b[2], ip_b[3]);

    // little-endian
    // snprintf(buffer, 16, "%d.%d.%d.%d", ip_b[3], ip_b[2], ip_b[1], ip_b[0]);

    return std::string(buffer);
}

