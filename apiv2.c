#ifdef DEBUG
    #include <stdio.h>
#endif
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
#include "headers/apiv2.h"

/* This is a TEST APIv2 Client for osu.ppy.sh */

const char *token = "";

/* 
To get token:

curl -X POST "https://osu.ppy.sh/oauth/token" -H "Accept: application/json" -H "Content-Type: application/json" -d '{"client_id":yourid,"client_secret":"your secret key","grant_type":"client_credentials","scope":"public"}' 

Find client id and client secret from osu settings page
*/

volatile int retrys = 0;

int osu_apiv2(struct beatmap *attributes, int beatmap_id, int mods){
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
        return -1;
    }

    memmove(&serv.sin_addr.s_addr, host->h_addr_list[0], sizeof(host->h_length));
    
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0){
        fprintf(stderr, "socket: %s\n", strerror(errno));
        return -1;
    }

    if(connect(fd, (struct sockaddr *)&serv, (socklen_t)sizeof(serv)) < 0){
        fprintf(stderr, "connect: %s\n", strerror(errno));
        exit(1);
    }

    /* Grab beatmap attributes */

    sprintf(request1, "POST /api/v2/beatmaps/%d/attributes HTTP/1.1\r\nHost: osu.ppy.sh\r\nAuthorization: Bearer %s\r\nContent-Type: application/json\r\nAccept: application/json\r\nContent-Length: 28\r\n\r\n{\"mods\":%d,\"ruleset\":\"osu\"}\r\n", beatmap_id, token, mods);
    
    /* Send request 1 to osu.ppy.sh */
    if(send(fd, request1, strlen(request1), 0) < 0){
        perror("send()");
        return -1;
    }

    if(recv(fd, buffer, 5000, 0) < 0){
        perror("recv()");
        return -1;
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
    MAX retrys is set to 10 
    */
retry: memset(request2, '\0', 2000); 
       memset(buffer2, '\0', 10000);
       memset(additional_beatmap_info, '\0', 10000);

    sprintf(request2, "GET /api/v2/beatmaps/lookup?id=%d HTTP/1.1\r\nHost: osu.ppy.sh\r\nAuthorization: Bearer %s\r\nContent-Type: application/json\r\nAccept: application/json\r\n\r\n", beatmap_id, token);

    /* Send request 2 to osu.ppy.sh */
    if(send(fd, request2, strlen(request2), 0) < 0){
        perror("send()");
        return -1;
    }

    if(recv(fd, buffer2, 10000, 0) < 0){
        perror("recv");
        return -1;
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
            printf("Retrys: %d\n", retrys);
        #endif
        if(retrys > 10){
            return -1;
        }
        goto retry;
    }

    close(fd);
    
    return attributes->countcircle == 0 ? -1 : 1;
}
