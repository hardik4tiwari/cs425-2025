# TCP Three-Way Handshake using Raw Sockets

##  Requirements

- Linux environment (raw sockets require root access)
- C++17 or later
- g++ compiler
- Root privileges to run the program

---

##  Build Instructions

- open linux terminal in this folder
- run command:  sudo make

## How to run the program

- Run the server (in one terminal): sudo make run-server

- Run the client (in another terminal): sudo make run-client

## Code flow
This program simulates the **TCP three-way handshake** (SYN, SYN-ACK, and ACK) by crafting and sending raw TCP packets using raw sockets. The raw socket allows for creating custom headers and sending packets with specific fields, allowing us to bypass some of the standard OS-level networking features and have full control over the packet structure.

The three steps of the TCP handshake are:

1. **SYN**: The client initiates the handshake by sending a SYN packet to the server.
2. **SYN-ACK**: The server responds with a SYN-ACK packet, acknowledging the received SYN.
3. **ACK**: The client acknowledges the server's SYN-ACK with an ACK packet, completing the handshake.

## Step-by-Step Breakdown

### 1. **Include Necessary Libraries**

```cpp
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
```
These headers provide the necessary functions and data structures for socket programming, TCP/IP handling, and checksum calculations:
- `<iostream>`: for printing output to the console.
- `<cstring>`: for string manipulations (e.g., `memcpy`).
- `<cstdlib>`: for general utility functions (e.g., `exit()`).
- `<unistd.h>`: for system calls like `close()` to close sockets.
- `<arpa/inet.h>`: for address conversion functions such as `inet_pton()` and `inet_addr()`.
- `<netinet/ip.h>`: for IP header structure.
- `<netinet/tcp.h>`: for TCP header structure.
- `<sys/socket.h>`: for socket programming functions.

### 2. **Define Constants and Pseudo-Header Structure**

```cpp
#define SERVER_PORT 12345
#define SOURCE_PORT 99999  //source port
```
- `SERVER_PORT`: Defines the destination port number for the server to listen to.
- `SOURCE_PORT`: Defines the source port number for the client.

```cpp
struct pseudo_header {
    uint32_t src_addr;
    uint32_t dst_addr;
    uint8_t placeholder;
    uint8_t protocol;
    uint16_t tcp_length;
};
```
The **pseudo header** is used in the TCP checksum calculation. It is part of the checksum process but does not actually appear in the final packet. It includes:
- `src_addr`: Source IP address.
- `dst_addr`: Destination IP address.
- `placeholder`: A placeholder byte (unused).
- `protocol`: Protocol type (TCP).
- `tcp_length`: Length of the TCP header.

### 3. **Checksum Calculation Function**

```cpp
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
```
This function calculates the checksum for both the IP and TCP headers. Checksums are crucial for error-checking in the network communication process. The function works by:
- Adding the 16-bit words in the data.
- Handling overflow using the carry operation.
- Finalizing the checksum by inverting the result.

### 4. **Crafting the TCP Packet**

```cpp
void craft_tcp_packet(char *packet, uint32_t src_ip, uint32_t dst_ip, uint32_t seq, uint32_t ack_seq,
                      bool syn_flag, bool ack_flag) {
    // IP header
    struct iphdr *ip = (struct iphdr *)packet;
    struct tcphdr *tcp = (struct tcphdr *)(packet + sizeof(struct iphdr));
    // Set IP header fields and TCP header fields
    // Calculate checksum for IP and TCP headers
}
```
This function constructs the TCP packet by:
- Filling in the **IP header** fields (e.g., source IP, destination IP, TTL, protocol, etc.).
- Filling in the **TCP header** fields (e.g., source port, destination port, sequence numbers, acknowledgment numbers, and flags like SYN, ACK).
- Calculating the checksum for both the IP and TCP headers.

### 5. **Setting Up the Raw Socket**

```cpp
int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
if (sock < 0) {
    perror("Socket creation failed");
    return 1;
}
```
- This creates a raw socket using `socket()`, where `AF_INET` specifies an IPv4 socket, `SOCK_RAW` specifies raw packets, and `IPPROTO_TCP` indicates the protocol (TCP) we are working with.

```cpp
int one = 1;
if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
    perror("setsockopt() failed");
    return 1;
}
```
- This ensures the IP header is included in the packet and not generated by the operating system. Without this, the OS would automatically add an IP header to the packet.

### 6. **Craft and Send SYN Packet (Step 1: SYN)**

```cpp
char packet[4096];
memset(packet, 0, sizeof(packet));
craft_tcp_packet(packet, src_ip, dst_ip, 200, 0, true, false);
sendto(sock, packet, sizeof(struct iphdr) + sizeof(struct tcphdr), 0, (struct sockaddr *)&dest, sizeof(dest));
std::cout << "[+] Sent SYN (seq=200)
";
```
- This creates a **SYN packet** with a sequence number (`seq=200`), and no acknowledgment number (`ack_seq=0`). The TCP flags are set to `SYN` (using the `syn_flag = true` argument) and `ACK` is false.
- The packet is then sent using the `sendto()` function.

### 7. **Wait and Receive SYN-ACK (Step 2: SYN-ACK)**

```cpp
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
        std::cout << "[+] Received SYN-ACK
";
        server_seq = ntohl(tcp->seq);
        break;
    }
}
```
- The client waits for a **SYN-ACK** response from the server. This is done by listening for a packet with both the `SYN` and `ACK` flags set. Once received, the sequence number (`server_seq`) is extracted for use in the next packet.

### 8. **Send Final ACK (Step 3: ACK)**

```cpp
memset(packet, 0, sizeof(packet));
craft_tcp_packet(packet, src_ip, dst_ip, 600, server_seq + 1, false, true);
sendto(sock, packet, sizeof(struct iphdr) + sizeof(struct tcphdr), 0, (struct sockaddr *)&dest, sizeof(dest));
std::cout << "[+] Sent final ACK (seq=600, ack_seq=" << server_seq + 1 << ")
";
```
- After receiving the **SYN-ACK**, the client sends an **ACK packet** to finalize the handshake. The sequence number is set to `600`, and the acknowledgment number is set to `server_seq + 1` (acknowledging the server's sequence).

### 9. **Close the Socket**

```cpp
close(sock);
```
- After completing the handshake, the socket is closed to release the resources.


