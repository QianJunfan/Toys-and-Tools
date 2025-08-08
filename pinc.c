//      This is a simplified and concise ping tool written in C. Hope you like it. :)
//      Code it for fun. :)
//                                  On vacation in Yunnan, China, August 8, 2025

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/types.h>

#define PACKET_SIZE    64
#define MAX_WAIT_TIME  5
#define MAX_NO_PACKETS 3

unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int    sum = 0;
    unsigned short  result;
    
    for (sum = 0; len > 1; len -= 2) {
        sum += *buf++;
    }
    if (len == 1) {
        sum += *(unsigned char *)buf;
    }
    sum  = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);

    result = ~sum;
    return result;
}

void build_packet(struct icmp *icmp, int seq) {
    icmp -> icmp_type = ICMP_ECHO;
    icmp -> icmp_code = 0;
    icmp -> icmp_id = getpid();
    icmp -> icmp_seq = seq;

    gettimeofday((struct timeval *)icmp -> icmp_data, NULL);
    icmp -> icmp_cksum = checksum(icmp, sizeof(struct icmp));
}

void send_ping(int sockfd, struct sockaddr_in *addr, int seq) {
    char packet[PACKET_SIZE];

    struct icmp *icmp = (struct icmp *)packet;
    
    memset(packet, 0, PACKET_SIZE);
    build_packet(icmp, seq);
    
    if (sendto(sockfd, packet, sizeof(struct icmp),
               0, (struct sockaddr *)addr, sizeof(*addr)) <= 0) {
        perror("ERROR");
    }
}

int recv_ping(int sockfd, struct sockaddr_in *addr, int seq) {
    struct timeval tv;
    tv.tv_sec  = MAX_WAIT_TIME;
    tv.tv_usec = 0;
    
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    
    if (select(sockfd + 1, &readfds, NULL, NULL, &tv) <= 0) {
        return 0;
    }
    
    char packet[PACKET_SIZE];
    struct sockaddr_in r_addr;
    socklen_t addr_len = sizeof(r_addr);
    
    if (recvfrom(sockfd, packet, PACKET_SIZE, 0, (struct sockaddr *)&r_addr, &addr_len) <= 0) {
        perror("ERROR");
        return -1;
    }
    
    struct ip *ip = (struct ip *)packet;
    int hlen = ip -> ip_hl << 2;
    struct icmp *icmp = (struct icmp *)(packet + hlen);
    
    if (icmp->icmp_type != ICMP_ECHOREPLY || icmp->icmp_id != getpid()) {
        return -1;
    }
    
    struct timeval *t_sent = (struct timeval *)icmp->icmp_data;
    struct timeval t_recv;
    gettimeofday(&t_recv, NULL);
    
    double rtt = (t_recv.tv_sec - t_sent->tv_sec) * 1000.0 +
    (t_recv.tv_usec - t_sent->tv_usec) / 1000.0;
    
    printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
           ntohs(ip -> ip_len) - hlen,
           inet_ntoa(addr -> sin_addr),
           icmp -> icmp_seq,
           ip   -> ip_ttl,
           rtt);
    
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <hostname/IP>\n", argv[0]);
        return 1;
    }
    
    struct hostent *host;
    struct sockaddr_in addr;
    
    if ((host = gethostbyname(argv[1])) == NULL) {
        herror("ERROR");
        return 1;
    }
    
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = host->h_addrtype;
    addr.sin_port = 0;
    memcpy(&addr.sin_addr, host->h_addr, host->h_length);
    
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("ERROR");
        return 1;
    }
    
    printf("PING %s (%s): %lu data bytes\n",
           argv[1], inet_ntoa(addr.sin_addr), (unsigned long)(PACKET_SIZE - sizeof(struct icmp)));
    
    for (int i = 0; i < MAX_NO_PACKETS; i++) {
        send_ping(sockfd, &addr, i);
        if (recv_ping(sockfd, &addr, i) < 0) {
            printf("Request timeout for icmp_seq %d\n", i);
        }
        sleep(1);
    }
    
    close(sockfd);
    return 0;
}
