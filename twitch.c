#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <netdb.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include "headers/apiv2.h"
#include "headers/computepp.h"
#include "headers/parse.h"

float calcTotal(struct beatmap_data *, struct beatmap *);

/* Creates socket for twitch bot */
int twitch_socket(){
    struct sockaddr_in serv = {0};
    struct hostent *host = {0};

    if((host = gethostbyname("irc.chat.twitch.tv")) < 0){
        perror("gethostbyname()");
        exit(1);
    }

    serv.sin_family = AF_INET;
    serv.sin_port = htons(6667);
    memmove(&serv.sin_addr.s_addr, host->h_addr_list[0], sizeof(host->h_length));

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0){
        perror("socket()");
        exit(1);
    }

    if(connect(fd, (struct sockaddr *)&serv, (socklen_t)sizeof(serv)) < 0){
        perror("socket()");
        exit(1);
    }

    return fd;
}
/* Send oauth information to twitch irc */
void twitch_login(int fd, char *channel, char *botname, char *oauth){
    /* Wait */
    char pass[strlen(oauth)];
    char buffer[strlen(botname)];
    char chan[strlen(channel)];
    char response[255];

    usleep(7500);
    sprintf(pass, "PASS oauth:%s\r\n", oauth);
    if(write(fd, pass, strlen(pass)) < 0){
        perror("write()");
        exit(1);
    }

    usleep(7500);
    sprintf(buffer, "NICK %s\r\n", botname);
    if(write(fd, buffer, strlen(buffer)) < 0){
        perror("write()");
        exit(1);
    }

    usleep(7500);
    sprintf(chan, "JOIN #%s\r\n", channel);
    if(write(fd, chan, strlen(chan)) < 0){
        perror("write()");
        exit(1);
    }

    /* check for invalid information */
    memset(response, '\0', 255);
    if(recv(fd, response, 255, SOCK_NONBLOCK) < 0){
        perror("read()");
        exit(1);
    }

    if(!strncmp(response, ":tmi.twitch.tv NOTICE * :Login authentication failed", 52)){
        printf("Login authentication failed.\n\n");
        exit(1);
    }

    puts(response);
}
/* Periodically check pulse */
void ping_check(int fd, char *twitch_chat){
    if(!strncmp(twitch_chat, "PING", 4)){
        if(send(fd, "PONG :tmi.twitch.tv\r\n", strlen("PONG :tmi.twitch.tv\r\n"), 0) < 0){
            perror("send()");
            exit(1);
        }
    }
}
/* Command check function */
void commands(int fd, char *chat, char *channel, struct beatmap *attributes, struct beatmap_data *data){
    char user[50] = {'\0'};
    char command[5] = {'\0'};
    char mod[10] = {'\0'};
    int beatmap = 0;
    int mods = 0;

    sscanf(chat, "%s %s %d %s", user, command, &beatmap, mod);

    if(!strncmp("!pp", command, 3)){
        if(!strncmp(mod, "+dt", 4)){
            mods = MODS_DT;
        } else if(!strncmp(mod, "+dthr", 6) || !strncmp(mod, "+hrdt", 6)){
            mods = MODS_DT | MODS_HR;
        } else if(!strncmp(mod, "+dthd", 6) || !strncmp(mod, "+hddt", 6)){
            mods = MODS_DT | MODS_HD;
        } else if(!strncmp(mod, "+dthrhd", 8) || !strncmp(mod, "+hrdthd", 8) || !strncmp(mod, "+hddthr", 8) || !strncmp(mod, "+hrhddt", 8)){
            mods = MODS_DT | MODS_HR | MODS_HD;
        } else {
            mods = MODS_NOMOD;
        }

         /* Get information about beatmap from osu apiv2 */
        int ret = osu_apiv2(attributes, beatmap, mods);
        if(ret < 0){
            char err[255];
            printf("\033[0;35mosu api failed!?\033[0m\n");
            memset(err, '\0', 255);
            sprintf(err, "PRIVMSG #%s :Failed to retrieve beatmap!\r\n", channel);
            send(fd, err, strlen(err), 0);
            return;
        }

        #ifdef DEBUG
            printf("\nStar Rating: \033[1;33m%.2f\033[0m\nMax Combo: \033[1;32m%d\033[0m\nAim: \033[1;36m%.2f\033[0m\nSpeed: \033[1;31m%.2f\033[0m\nSpeed Note Count: \033[0m%.2f\nFlashlight: \033[1;34m%.2f\033[0m\nSlider Factor: %.2f\nAR: \033[0;35m%.2f\033[0m\nOD: \033[1;35m%.2f\033[0m\n\n", attributes->stars, attributes->maxcombo, attributes->aim, attributes->speed, attributes->speednotecount, attributes->flashlight, attributes->sliderfactor, attributes->ar, attributes->od);
            printf("Count Circles: %d\nCount Sliders: %d\nCount Spinners %d\n\n", attributes->countcircle, attributes->countsliders, attributes->countspinners);
        #endif

        /* Calc beatmaps PP with current osu performance point formula */
        float pp = calcTotal(data, attributes);

        #ifdef DEBUG
            printf("PP:\033[1;31m %.2f\033[0m\n\n", pp);
        #endif

        /* Write to twitch chat here */
        char response[255];
        memset(response, '\0', 255);
        sprintf(response, "PRIVMSG #%s :PP->%.2f\r\n", channel, pp);
        send(fd, response, strlen(response), 0);
    }
}
