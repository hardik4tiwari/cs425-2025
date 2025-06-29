#include <iostream>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_PORT 12345  // Listening port

void print_tcp_flags(struct tcphdr *tcp) {
    std::cout << "[+] TCP Flags: "
              << " SYN: " << tcp->syn
              << " ACK: " << tcp->ack
              << " FIN: " << tcp->fin
              << " RST: " << tcp->rst
              << " PSH: " << tcp->psh
              << " SEQ: " << ntohl(tcp->seq) << std::endl;
}

void send_syn_ack(int sock, struct sockaddr_in *client_addr, struct tcphdr *tcp) {
    char packet[sizeof(struct iphdr) + sizeof(struct tcphdr)];
    memset(packet, 0, sizeof(packet));

    struct iphdr *ip = (struct iphdr *)packet;
    struct tcphdr *tcp_response = (struct tcphdr *)(packet + sizeof(struct iphdr));

    // Fill IP header
    ip->ihl = 5;
    ip->version = 4;
    ip->tos = 0;
    ip->tot_len = htons(sizeof(packet));
    ip->id = htons(54321);
    ip->frag_off = 0;
    ip->ttl = 64;
    ip->protocol = IPPROTO_TCP;
    ip->saddr = client_addr->sin_addr.s_addr;
    ip->daddr = inet_addr("127.0.0.1");  // Server address

    // Fill TCP header
    tcp_response->source = tcp->dest;
    tcp_response->dest = tcp->source;
    tcp_response->seq = htonl(400);
    tcp_response->ack_seq = htonl(ntohl(tcp->seq) + 1);
    tcp_response->doff = 5;
    tcp_response->syn = 1;
    tcp_response->ack = 1;
    tcp_response->window = htons(8192);
    tcp_response->check = 0;  // Kernel will compute the checksum

    // Send packet
    if (sendto(sock, packet, sizeof(packet), 0, (struct sockaddr *)client_addr, sizeof(*client_addr)) < 0) {
        perror("sendto() failed");
    } else {
        std::cout << "[+] Sent SYN-ACK" << std::endl;
    }
}

void receive_syn() {
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Enable IP header inclusion
    int one = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt() failed");
        exit(EXIT_FAILURE);
    }

    char buffer[65536];
    struct sockaddr_in source_addr;
    socklen_t addr_len = sizeof(source_addr);

    while (true) {
        int data_size = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&source_addr, &addr_len);
        if (data_size < 0) {
            perror("Packet reception failed");
            continue;
        }

        struct iphdr *ip = (struct iphdr *)buffer;
        struct tcphdr *tcp = (struct tcphdr *)(buffer + (ip->ihl * 4));

        // Only process packets for the correct destination port
        if (ntohs(tcp->dest) != SERVER_PORT) continue;

        print_tcp_flags(tcp);

        if (tcp->syn == 1 && tcp->ack == 0 && ntohl(tcp->seq) == 200) {
            std::cout << "[+] Received SYN from " << inet_ntoa(source_addr.sin_addr) << std::endl;
            send_syn_ack(sock, &source_addr, tcp);
        }

        if (tcp->ack == 1 && tcp->syn == 0 && ntohl(tcp->seq) == 600) {
            std::cout << "[+] Received ACK, handshake complete." << std::endl;
            break;
        }
    }

    close(sock);
}

int main() {
    std::cout << "[+] Server listening on port " << SERVER_PORT << "..." << std::endl;
    receive_syn();
    return 0;
}

