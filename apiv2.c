#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include "headers/parse.h"
#include "headers/computepp.h"

/* This is a TEST APIv2 Client for osu.ppy.sh */

volatile int retrys = 0;

float calcTotal(struct beatmap_data *, struct beatmap *, int);

int get_token(char *osutoken){
    struct sockaddr_in serv = {0};
    struct hostent *host = {0};

    serv.sin_family = AF_INET;
    serv.sin_port = htons(80);

    if((host = gethostbyname("osu.ppy.sh")) < 0){
        fprintf(stderr, "gethostbyname: %s\n", strerror(errno));
        return 2;
    }

    memmove(&serv.sin_addr.s_addr, host->h_addr_list[0], sizeof(host->h_length));
    
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0){
        fprintf(stderr, "socket: %s\n", strerror(errno));
        return 2;
    }

    if(connect(fd, (struct sockaddr *)&serv, (socklen_t)sizeof(serv)) < 0){
        fprintf(stderr, "connect: %s\n", strerror(errno));
        close(fd);
        return 2;
    }

    char request[1024] = {0};
    char buffer[2024] = {0};
    
    memset(request, '\0', 1024);
    memset(buffer, '\0', 2024);
    sprintf(request, "POST /oauth/token HTTP/1.1\r\nHost: osu.ppy.sh\r\nAccept: application/json\r\nContent-Type: application/json\r\nContent-Length: 131\r\n\r\n{\"client_id\":%d,\"client_secret\":\"%s\",\"grant_type\":\"client_credentials\",\"scope\":\"public\"}\r\n", client_id, client_secret);
    
    if(send(fd, request, strlen(request), 0) < 0){
        perror("send(), get_token(), apiv2.c\n");
        close(fd);
        return -1;
    }
    if(recv(fd, buffer, 2024, 0) < 0){
        perror("recv(), get_token(), apiv2.c\n");
        close(fd);
        return -1;
    }

    memset(request, '\0', 1024);

    int found = 0;
    for(int i = 0, j = 0; i < strlen(buffer); i++){
        if(buffer[i] == 'e' && buffer[i+1] == 'n' && buffer[i+2] == '"' && buffer[i+3] == ':'){ // Triming response
            found = 1;
        }
        if(found == 1){
            if(buffer[i+5] == '"'){
                osutoken[j] = '\0';
                break;
            }
            osutoken[j] = buffer[i+5];
            j++;
        }
    }

    close(fd);
    return osutoken[0] == '\0' ? -1 : 0;
}

int osu_apiv2(struct beatmap *attributes, int beatmap_id, int mods, char *osutoken){
    struct sockaddr_in serv = {0};
    struct hostent *host = {0};
    char request1[2000], buffer[5000];

    retrys = 0;

    serv.sin_family = AF_INET;
    serv.sin_port = htons(80);

    memset(request1, '\0', 2000);
    memset(buffer, '\0', 5000);


    if((host = gethostbyname("osu.ppy.sh")) < 0){
        fprintf(stderr, "gethostbyname: %s\n", strerror(errno));
        return 2;
    }

    memmove(&serv.sin_addr.s_addr, host->h_addr_list[0], sizeof(host->h_length));
    
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0){
        fprintf(stderr, "socket: %s\n", strerror(errno));
        return 2;
    }

    if(connect(fd, (struct sockaddr *)&serv, (socklen_t)sizeof(serv)) < 0){
        fprintf(stderr, "connect: %s\n", strerror(errno));
        close(fd);
        return 2;
    }

    /* Grab beatmap attributes */

    sprintf(request1, "POST /api/v2/beatmaps/%d/attributes HTTP/1.1\r\nHost: osu.ppy.sh\r\nAuthorization: Bearer %s\r\nContent-Type: application/json\r\nAccept: application/json\r\nContent-Length: 28\r\n\r\n{\"mods\":%d,\"ruleset\":\"osu\"}\r\n", beatmap_id, osutoken, mods);
    
    /* Send request 1 to osu.ppy.sh */
    if(send(fd, request1, strlen(request1), 0) < 0){
        perror("send()");
        close(fd);
        return 2;
    }

    if(recv(fd, buffer, 5000, 0) < 0){
        perror("recv()");
        close(fd);
        return 2;
    }

    /* Parse response from request 1 */
    int found = 0;
    char beatmap_attributes[1000];
    memset(beatmap_attributes, '\0', 1000);

    for(int i = 0, j = 0; i < strlen(buffer); i++){
        if(buffer[i] == '{' && buffer[i+1] == '"'){ // Triming response
            found = 1;
        }
        if(found == 1){
            beatmap_attributes[j] = buffer[i];
            j++;
        }
    }

    /* Parsing all attributes with json-c */
    parse_attributes(beatmap_attributes, attributes);

    
    /* Grab beatmaps circle count, slider count and spinner count... */
    char request2[2000], buffer2[10000], additional_beatmap_info[10000];
    /* 
    Jump to "retry" IF the osu api doesn't return a circle count
    MAX retrys is set to 5
    */
    #ifdef DEBUG
        puts("");
    #endif
retry: memset(request2, '\0', 2000); 
       memset(buffer2, '\0', 10000);
       memset(additional_beatmap_info, '\0', 10000);

    sprintf(request2, "GET /api/v2/beatmaps/lookup?id=%d HTTP/1.1\r\nHost: osu.ppy.sh\r\nAuthorization: Bearer %s\r\nContent-Type: application/json\r\nAccept: application/json\r\n\r\n", beatmap_id, osutoken);

    /* Send request 2 to osu.ppy.sh */
    if(send(fd, request2, strlen(request2), 0) < 0){
        perror("send()2");
        close(fd);
        return 2;
    }

    if(recv(fd, buffer2, 10000, 0) < 0){
        perror("recv");
        close(fd);
        return 2;
    }

    /* Parse response from request 2 */
    found = 0;
    for(int i = 0, j = 0; i < strlen(buffer2); i++){
        if(buffer2[i] == '{' && buffer2[i+1] == '"'){ // Triming response
            found = 1;
        }
        if(found == 1){
            additional_beatmap_info[j] = buffer2[i];
            j++;
        }
    }
    
    /* Parsing additional attributes with json-c */
    parse_additional_info(additional_beatmap_info, attributes);

    if(attributes->countcircle == 0){ // If osu apiv2 fails to give a response then jump back to "retry"
        sleep(1);
        retrys++;
        #ifdef DEBUG
            char retryz[25];
            memset(retryz, '\0', 25);
            sprintf(retryz, "\rRetrys:\033[1;33m %d\033[0m  ", retrys);
            write(1, retryz, strlen(retryz));
            fflush(stdout);
        #endif
        if(retrys > 5){
            close(fd);
            return -1; // Failed to get beatmap attributes
        }
        goto retry;
    }

    close(fd);
    
    return attributes->countcircle == 0 ? -1 : 0;
}
