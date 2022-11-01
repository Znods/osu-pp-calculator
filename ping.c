#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>

#define PING_PKT_S 64
#define PORT_NO 0
#define PING_SLEEP_RATE 1000000
#define RECV_TIMEOUT 1

struct ping_pkt{
    struct icmphdr hdr;
    char msg[PING_PKT_S - sizeof(struct icmphdr)];
};

double ping_ms(int fd, struct sockaddr_in *ping_addr, char *ping_ip);

unsigned short checksum(void *b, int len){    
    unsigned short *buf = b;
    unsigned int sum=0;
    unsigned short result;
 
    for ( sum = 0; len > 1; len -= 2 )
        sum += *buf++;
    if ( len == 1 )
        sum += *(unsigned char*)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

double ping_socket(char *ip){
    int fd = 0;
    struct sockaddr_in serv = {0};
    struct hostent *host;

    if((host = gethostbyname(ip)) < 0){
        perror("gethostbyname() - ping_socket()\n");
        return -1;
    }

    serv.sin_family = AF_INET;
    serv.sin_port = htons(PORT_NO);
    memmove(&serv.sin_addr.s_addr, host->h_addr_list[0], sizeof(host->h_length));

    fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(fd < 0){
        perror("socket(), ping_socket\n");
        return -1;
    }
    double ms = ping_ms(fd, &serv, ip);
    close(fd);
    return ms;
}

double ping_ms(int fd, struct sockaddr_in *ping_addr, char *ping_ip){
    int ttl_val = 64, msg_count = 0, i, flag = 1, msg_received_count = 0;
    int retrys = 0;

    socklen_t addr_len;
     
    struct ping_pkt pckt;
    struct sockaddr_in r_addr;
    struct timespec time_start, time_end, tfs, tfe;

    long double rtt_msec = 0;

    struct timeval tv_out;

    tv_out.tv_sec = RECV_TIMEOUT;
    tv_out.tv_usec = 0;
 
    clock_gettime(CLOCK_MONOTONIC, &tfs);
 
     
    // set socket options at ip to TTL and value to 64,
    // change to what you want by setting ttl_val
    if (setsockopt(fd, SOL_IP, IP_TTL, &ttl_val, sizeof(ttl_val)) != 0){
        printf("\nSetting socket options to TTL failed!\n");
        return -1;
    }
 
    // setting timeout of recv setting
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_out, sizeof tv_out);
 
    // send icmp packet 
    // flag is whether packet was sent or not
retry:
    retrys++;
    if(retrys > 10){
        return -1;
    }

    flag = 1;
      
    //filling packet
    bzero(&pckt, sizeof(pckt));
         
    pckt.hdr.type = ICMP_ECHO;
    pckt.hdr.un.echo.id = getpid();
         
    for ( i = 0; i < sizeof(pckt.msg)-1; i++ )
        pckt.msg[i] = i+'0';
         
    pckt.msg[i] = 0;
    pckt.hdr.un.echo.sequence = msg_count++;
    pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));
 
 
    usleep(PING_SLEEP_RATE);
 
    // send packet
    clock_gettime(CLOCK_MONOTONIC, &time_start);

    if(sendto(fd, &pckt, sizeof(pckt), 0, (struct sockaddr*) ping_addr, sizeof(*ping_addr)) <= 0){
        printf("\nPacket Sending Failed!\n");
        flag = 0;
        goto retry;
    }
 
    // receive packet
    addr_len = sizeof(r_addr);
 
    if(recvfrom(fd, &pckt, sizeof(pckt), 0, (struct sockaddr*)&r_addr, &addr_len) <= 0 && msg_count>1){
        //printf("Packet receive failed!\n");
        ;
    } else {
        clock_gettime(CLOCK_MONOTONIC, &time_end);
             
        double timeElapsed = ((double)(time_end.tv_nsec - time_start.tv_nsec))/1000000.0;
        rtt_msec = (time_end.tv_sec - time_start.tv_sec) * 1000.0 + timeElapsed;
             
        // if packet was not sent, don't receive
        if(flag){
            if(!(pckt.hdr.type == 69 && pckt.hdr.code == 0)){ 
                printf("Error..Packet received with ICMP type %d code %d\n", pckt.hdr.type, pckt.hdr.code);
                goto retry;
            } else {
                #ifdef DEBUG
                    printf("\033[1;31m%d bytes from (%s) msg_seq=%d ttl=%d rtt = %Lf ms.\033[0m\n", PING_PKT_S, ping_ip, msg_count, ttl_val, rtt_msec);
                #endif
                msg_received_count++;
            }
        }
    }

    if((int)rtt_msec > 500){
        write(1, "ms too high re-ping!\n", 22);
        goto retry;
    }

    clock_gettime(CLOCK_MONOTONIC, &tfe);

    return rtt_msec;
}
