#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

#define SERVER_PORT 12345
#define SOURCE_PORT 99999  //source port

// Pseudo header for TCP checksum calculation
struct pseudo_header {
    uint32_t src_addr;
    uint32_t dst_addr;
    uint8_t placeholder;
    uint8_t protocol;
    uint16_t tcp_length;
};

unsigned short calculate_checksum(unsigned short *ptr, int nbytes) {
    long sum = 0;
    while (nbytes > 1) {
        sum += *ptr++;
        nbytes -= 2;
    }
    if (nbytes == 1)
        sum += *((unsigned char *)ptr);
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

void craft_tcp_packet(char *packet, uint32_t src_ip, uint32_t dst_ip, uint32_t seq, uint32_t ack_seq,
                      bool syn_flag, bool ack_flag) {
    // IP header
    struct iphdr *ip = (struct iphdr *)packet;
    struct tcphdr *tcp = (struct tcphdr *)(packet + sizeof(struct iphdr));

    ip->ihl = 5;
    ip->version = 4;
    ip->tos = 0;
    ip->tot_len = htons(sizeof(struct iphdr) + sizeof(struct tcphdr));
    ip->id = htons(99999);
    ip->frag_off = 0;
    ip->ttl = 64;
    ip->protocol = IPPROTO_TCP;
    ip->check = 0;
    ip->saddr = src_ip;
    ip->daddr = dst_ip;
    ip->check = calculate_checksum((unsigned short *)ip, sizeof(struct iphdr));

    // TCP header
    tcp->source = htons(SOURCE_PORT);
    tcp->dest = htons(SERVER_PORT);
    tcp->seq = htonl(seq);
    tcp->ack_seq = htonl(ack_seq);
    tcp->doff = 5;
    tcp->syn = syn_flag;
    tcp->ack = ack_flag;
    tcp->window = htons(8192);
    tcp->check = 0;

    // Pseudo header + TCP header for checksum
    struct pseudo_header psh;
    psh.src_addr = src_ip;
    psh.dst_addr = dst_ip;
    psh.placeholder = 0;
    psh.protocol = IPPROTO_TCP;
    psh.tcp_length = htons(sizeof(struct tcphdr));

    char pseudo_packet[sizeof(struct pseudo_header) + sizeof(struct tcphdr)];
    memcpy(pseudo_packet, &psh, sizeof(struct pseudo_header));
    memcpy(pseudo_packet + sizeof(struct pseudo_header), tcp, sizeof(struct tcphdr));

    tcp->check = calculate_checksum((unsigned short *)pseudo_packet, sizeof(pseudo_packet));
}

int main() {
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    int one = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt() failed");
        return 1;
    }

    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, "127.0.0.1", &(dest.sin_addr));

    uint32_t src_ip = inet_addr("127.0.0.1");
    uint32_t dst_ip = inet_addr("127.0.0.1");

    // Step 1: Send SYN (seq = 200)
    char packet[4096];
    memset(packet, 0, sizeof(packet));
    craft_tcp_packet(packet, src_ip, dst_ip, 200, 0, true, false);
    sendto(sock, packet, sizeof(struct iphdr) + sizeof(struct tcphdr), 0, (struct sockaddr *)&dest, sizeof(dest));
    std::cout << "[+] Sent SYN (seq=200)\n";

    // Step 2: Receive SYN-ACK
    char buffer[65536];
    struct sockaddr_in from;
    socklen_t from_len = sizeof(from);
    uint32_t server_seq = 0;

    while (true) {
        int len = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&from, &from_len);
        if (len < 0) {
            perror("recvfrom failed");
            continue;
        }

        struct iphdr *ip = (struct iphdr *)buffer;
        struct tcphdr *tcp = (struct tcphdr *)(buffer + ip->ihl * 4);

        if (tcp->source == htons(SERVER_PORT) && tcp->dest == htons(SOURCE_PORT) && tcp->syn && tcp->ack) {
            std::cout << "[+] Received SYN-ACK\n";
            server_seq = ntohl(tcp->seq);
            break;
        }
    }

    // Step 3: Send ACK (seq = 600, ack_seq = server_seq + 1)
    memset(packet, 0, sizeof(packet));
    craft_tcp_packet(packet, src_ip, dst_ip, 600, server_seq + 1, false, true);
    sendto(sock, packet, sizeof(struct iphdr) + sizeof(struct tcphdr), 0, (struct sockaddr *)&dest, sizeof(dest));
    std::cout << "[+] Sent final ACK (seq=600, ack_seq=" << server_seq + 1 << ")\n";

    close(sock);
    return 0;
}
