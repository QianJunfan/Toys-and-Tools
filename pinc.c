//      This is a simplified and concise ping tool written in C. Hope you like it. :)
//      Code it for fun. :)
//                                       On vacation in Yunnan, China, August 8, 2025

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
#include <stdint.h>
#include <math.h>

#define PACKET_SIZE    64
#define MAX_WAIT_TIME  2
#define MAX_NO_PACKETS 36

void pinc_print(double a, uint32_t bg_color) {
    uint8_t bg_r = (bg_color >> 16) & 0xFF;
    uint8_t bg_g = (bg_color >> 8)  & 0xFF;
    uint8_t bg_b = bg_color & 0xFF;
    uint8_t fg_r = 0;
    uint8_t fg_g = 0;
    uint8_t fg_b = 0;
    printf("\x1b[48;2;%d;%d;%dm\x1b[38;2;%d;%d;%dm",
           bg_r, bg_g, bg_b, fg_r, fg_g, fg_b);

    printf(" ");
    
    if (a >= 1.0) {
        printf("%-5d", (int)round(a));
    } else {
        printf("%-5.3f", a);
    }

    printf(" ");
    
    printf("\x1b[0m");
}

void pinc_print_timeout(void) {
    uint32_t bg_color = 0xFF0000;
    uint8_t bg_r = (bg_color >> 16) & 0xFF;
    uint8_t bg_g = (bg_color >> 8)  & 0xFF;
    uint8_t bg_b = bg_color & 0xFF;
    uint8_t fg_r = 0;
    uint8_t fg_g = 0;
    uint8_t fg_b = 0;

    printf("\x1b[48;2;%d;%d;%dm\x1b[38;2;%d;%d;%dm",
           bg_r, bg_g, bg_b, fg_r, fg_g, fg_b);
    
    // Print "XXXXX" with a leading and trailing space
    printf(" XXXXX ");
    
    printf("\x1b[0m");
}

uint32_t get_gradient_color(double value) {
    if (value <= 20) {
        return 0x00FF00;
    }
    if (value >= 500) {
        return 0xFF0000;
    }
    uint8_t red, green;
    if (value <= 100) {
        red   = (uint8_t)((value - 20) * 255.0 / (100 - 20));
        green = 255;
    } else {
        red   = 255;
        green = (uint8_t)(255 - (value - 100) * 255.0 / (500 - 100));
    }
    return (red << 16) | (green << 8);
}

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
    icmp -> icmp_id   = getpid();
    icmp -> icmp_seq  = seq;
    gettimeofday((struct timeval *)icmp -> icmp_data, NULL);
    icmp -> icmp_cksum = checksum(icmp, sizeof(struct icmp));
}

void send_ping(int sockfd, struct sockaddr_in *addr, int seq) {
    char packet[PACKET_SIZE];
    struct icmp *icmp = (struct icmp *)packet;
    memset(packet, 0, PACKET_SIZE);
    build_packet(icmp, seq);
    if (sendto(sockfd, packet, PACKET_SIZE,
               0, (struct sockaddr *)addr, sizeof(*addr)) <= 0) {
        perror("ERROR");
    }
}

double recv_ping(int sockfd, struct sockaddr_in *addr, int seq, int *ttl_out) {
    struct timeval tv;
    tv.tv_sec  = MAX_WAIT_TIME;
    tv.tv_usec = 0;
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    if (select(sockfd + 1, &readfds, NULL, NULL, &tv) <= 0) {
        if (ttl_out) {
            *ttl_out = 0;
        }
        return 0.0;
    }
    char packet_buffer[PACKET_SIZE * 2];
    struct sockaddr_in r_addr;
    socklen_t addr_len = sizeof(r_addr);
    ssize_t bytes_received = recvfrom(sockfd, packet_buffer, sizeof(packet_buffer), 0, (struct sockaddr *)&r_addr, &addr_len);
    if (bytes_received <= 0) {
        perror("ERROR");
        if (ttl_out) {
            *ttl_out = 0;
        }
        return -1.0;
    }
    struct ip *ip_header = (struct ip *)packet_buffer;
    int hlen = ip_header -> ip_hl << 2;
    if (bytes_received < hlen + (int)sizeof(struct icmp)) {
        if (ttl_out) {
            *ttl_out = 0;
        }
        return -1.0;
    }
    struct icmp *icmp = (struct icmp *)(packet_buffer + hlen);
    if (icmp->icmp_type != ICMP_ECHOREPLY || icmp->icmp_id != getpid()) {
        if (ttl_out) {
            *ttl_out = 0;
        }
        return -1.0;
    }
    struct timeval *t_sent = (struct timeval *)icmp->icmp_data;
    struct timeval t_recv;
    gettimeofday(&t_recv, NULL);
    double rtt = (t_recv.tv_sec - t_sent->tv_sec) * 1000.0 + (t_recv.tv_usec - t_sent->tv_usec) / 1000.0;
    
    if (ttl_out) {
        *ttl_out = ip_header->ip_ttl;
    }

    return rtt;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <hostname/IP>\n", argv[0]);
        return 1;
    }
    
    char *hostname_str = argv[1];
    if (strstr(hostname_str, "://") != NULL) {
        hostname_str = strstr(hostname_str, "://") + 3;
    }

    struct hostent *host;
    struct sockaddr_in addr;
    if ((host = gethostbyname(hostname_str)) == NULL) {
        herror("ERROR");
        printf("ERROR: Unknown host\n");
        return 1;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = host->h_addrtype;
    addr.sin_port   = 0;
    memcpy(&addr.sin_addr, host->h_addr, host->h_length);
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("ERROR");
        return 1;
    }
    
    int packets_sent     = 0;
    int packets_received = 0;
    double total_rtt     = 0.0;
    double min_rtt       = -1.0;
    double max_rtt       = -1.0;
    int last_ttl         = -1;

    printf("\n\n     PINC \033[1m%s\033[0m (%s): %lu data bytes\n\n       ",
           argv[1], inet_ntoa(addr.sin_addr), (unsigned long)(PACKET_SIZE - sizeof(struct icmp)));
    
    for (int i = 0; i < MAX_NO_PACKETS; i++) {
        send_ping(sockfd, &addr, i);
        double rtt_val = recv_ping(sockfd, &addr, i, &last_ttl);
        
        packets_sent++;

        if (rtt_val > 0.0) {
            packets_received++;
            total_rtt += rtt_val;
            
            if (min_rtt == -1.0 || rtt_val < min_rtt) {
                min_rtt = rtt_val;
            }
            if (max_rtt == -1.0 || rtt_val > max_rtt) {
                max_rtt = rtt_val;
            }

            pinc_print(rtt_val, get_gradient_color(rtt_val));
        } else {
            pinc_print_timeout();
        }
        
        if ((i + 1) % 6 == 0) {
            printf("\n       ");
        }
        
        fflush(stdout);
        sleep(1);
    }

    if (MAX_NO_PACKETS % 6 != 0) {
        printf("\n");
    }

    close(sockfd);
    
    // Bold the subtitle and key information
    printf("\n\n     \033[1m%s PINC statistics\033[0m\n\n", argv[1]);
    printf("       \033[1mIP Address:\033[0m %s\n", inet_ntoa(addr.sin_addr));
    printf("       \033[1mPacket Size:\033[0m %lu bytes\n", (unsigned long)(PACKET_SIZE - sizeof(struct icmp)));
    printf("       \033[1mPackets Transmitted:\033[0m %d\n", packets_sent);
    printf("       \033[1mPackets Received:\033[0m %d\n", packets_received);
    
    if (packets_sent > 0) {
        printf("       \033[1mPacket Loss:\033[0m %.1f%%\n",
               (double)(packets_sent - packets_received) / packets_sent * 100.0);
    } else {
        printf("       \033[1mPacket Loss:\033[0m 0.0%%\n");
    }
    
    if (packets_received > 0) {
        double avg_rtt = total_rtt / packets_received;
        printf("       \033[1mRound-trip min/avg/max:\033[0m %.3f/%.3f/%.3f ms\n",
               min_rtt, avg_rtt, max_rtt);
        printf("       \033[1mLast TTL:\033[0m %d\n", last_ttl);
    } else {
        printf("       \033[1mLast TTL:\033[0m N/A\n");
    }
    printf("       \n\033[1m                                             - Good Bye :)\033[0m\n");
    printf("\n\n");

    return 0;
}
