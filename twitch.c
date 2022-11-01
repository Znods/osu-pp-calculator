#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include <netdb.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include "headers/mods.h"
#include "headers/computepp.h"
#include "headers/parse.h"

#define OWNER ":znods:" /* For "!s" command to close bot gracefully */

float calcTotal(struct beatmap_data *, struct beatmap *, int);
double ping_socket(char *);

/* Creates socket for twitch bot */
int twitch_socket(){
    struct sockaddr_in serv = {0};
    struct hostent *host = {0};

    if((host = gethostbyname("irc.chat.twitch.tv")) < 0){
        perror("gethostbyname()");
        return -1;
    }

    serv.sin_family = AF_INET;
    serv.sin_port = htons(6667);
    memmove(&serv.sin_addr.s_addr, host->h_addr_list[0], sizeof(host->h_length));

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0){
        perror("socket()");
        return -1;
    }

    if(connect(fd, (struct sockaddr *)&serv, (socklen_t)sizeof(serv)) < 0){
        perror("socket()");
        return -1;
    }

    return fd;
}
/* Send oauth information to twitch irc */
int twitch_login(int fd, char *channel, char *botname, char *oauth){
    char pass[strlen(oauth)], buffer[strlen(botname)],
    chan[strlen(channel)], response[255];

    usleep(7500);
    sprintf(pass, "PASS oauth:%s\r\n", oauth);
    if(write(fd, pass, strlen(pass)) < 0){
        perror("write() oauth:");
        return 2;
    }

    usleep(7500);
    sprintf(buffer, "NICK %s\r\n", botname);
    if(write(fd, buffer, strlen(buffer)) < 0){
        perror("write() nick:");
        return 2;
    }

    usleep(7500);
    sprintf(chan, "JOIN #%s\r\n", channel);
    if(write(fd, chan, strlen(chan)) < 0){
        perror("write() join:");
        return 2;
    }

    /* check for invalid information */
    memset(response, '\0', 255);
    if(recv(fd, response, 255, SOCK_NONBLOCK) < 0){
        perror("read() respone:");
        return 2;
    }

    if(!strncmp(response, ":tmi.twitch.tv NOTICE * :Login authentication failed", 52)){
        printf("Login authentication failed.\n\n");
        return -1;
    }

    #ifdef DEBUG
        write(1, response, strlen(response));
    #endif

    return 0;
}
/* Periodically check pulse */
int ping_check(int fd, char *twitch_chat){
    if(!strncmp(twitch_chat, "PING", 4)){
        if(send(fd, "PONG :tmi.twitch.tv\r\n", strlen("PONG :tmi.twitch.tv\r\n"), 0) < 0){
            perror("send()");
            return -1;
        }
        #ifdef DEBUG
            puts("\033[0;35mSent Ping!\033[0m");
        #endif
    }
    return 0;
}
/* Command check function */
bool commands(int fd, char *chat, char *channel, struct beatmap *attributes, struct beatmap_data *data, char *osutoken){
    char user[50] = {'\0'}, command[5] = {'\0'}, mod[10] = {'\0'};
    int beatmap = 0, mods = 0;

    memset(user, '\0', 50), memset(command, '\0', 5), memset(mod, '\0', 10);

    /* Parse Twitch Chat Command */
    sscanf(chat, "%s %s %d %s", user, command, &beatmap, mod);

    /* !pp command */
    if(!strncmp("!pp", command, 4)){
        if(!strncmp(mod, "+dt", 4)){
            mods = MODS_DT;
        } else if(!strncmp(mod, "+hr", 4)){
            mods = MODS_HR;
        } else if(!strncmp(mod, "+hd", 4)){
            mods = MODS_HD;
        } else if(!strncmp(mod, "+fl", 4)){
            mods = MODS_FL;
        } else if(!strncmp(mod, "+ez", 4)){
            mods = MODS_EZ;
        } else if(!strncmp(mod, "+ht", 4)){
            mods = MODS_HT;
        } else if(!strncmp(mod, "+dtez", 6) || !strncmp(mod, "+ezdt", 6)){
            mods = MODS_DT | MODS_EZ;
        } else if(!strncmp(mod, "+dthr", 6) || !strncmp(mod, "+hrdt", 6)){
            mods = MODS_DT | MODS_HR;
        } else if(!strncmp(mod, "+dthd", 6) || !strncmp(mod, "+hddt", 6)){
            mods = MODS_DT | MODS_HD;
        } else if(!strncmp(mod, "+hrhd", 6) || !strncmp(mod, "+hdhr", 6)){
            mods = MODS_HR | MODS_HD;
        } else if(!strncmp(mod, "+fldt", 6) || !strncmp(mod, "+dtfl", 6)){
            mods = MODS_DT | MODS_FL;
        } else if(!strncmp(mod, "+flhr", 6) || !strncmp(mod, "+hrfl", 6)){
            mods = MODS_HR | MODS_FL;
        } else if(!strncmp(mod, "+flhd", 6) || !strncmp(mod, "+hdfl", 6)){
            mods = MODS_HD | MODS_FL;
        } else if(!strncmp(mod, "+dthrhd", 8) || !strncmp(mod, "+dthdhr", 8) || !strncmp(mod, "+hddthr", 8) || !strncmp(mod, "+hrdthd", 8) || !strncmp(mod, "+hrhddt", 8) || !strncmp(mod, "+hdhrdt", 8)){
            mods = MODS_DT | MODS_HR | MODS_HD;
        } else if(!strncmp(mod, "+dthrfl", 8) || !strncmp(mod, "+dtflhr", 8) || !strncmp(mod, "+fldthr", 8) || !strncmp(mod, "+hrdtfl", 8) || !strncmp(mod, "+hrfldt", 8) || !strncmp(mod, "+flhrdt", 8)){
            mods = MODS_DT | MODS_HR | MODS_FL;
        } else if(!strncmp(mod, "+hrhdfl", 8) || !strncmp(mod, "+hrflhd", 8) || !strncmp(mod, "+flhrhd", 8) || !strncmp(mod, "+hdhrfl", 8) || !strncmp(mod, "+hdflhr", 8) || !strncmp(mod, "+flhdhr", 8)){
            mods = MODS_HD | MODS_HR | MODS_FL;
        } else if(!strncmp(mod, "+dthdfl", 8) || !strncmp(mod, "+dtflhd", 8) || !strncmp(mod, "+fldthd", 8) || !strncmp(mod, "+hddtfl", 8) || !strncmp(mod, "+hdfldt", 8) || !strncmp(mod, "+flhddt", 8)){
            mods = MODS_HD | MODS_DT | MODS_FL;
        } else {
            mods = MODS_NOMOD;
        }

         /* Get information about beatmap from osu apiv2 */
        int ret = osu_apiv2(attributes, beatmap, mods, osutoken);
        if(ret == -1){
            char err[70] = {0};
            #ifdef DEBUG
                printf("\033[0;35mosu api failed!?\033[0m\n");
            #endif
            sprintf(err, "PRIVMSG #%s :Failed to retrieve beatmap!\r\n", channel);
            send(fd, err, strlen(err), 0);
            return true;
        } else if(ret == 2){
            fprintf(stderr, "\n\nERR: in function osu_apiv2() | <%s>\n\n", __FILE__);
            return false;
        }

        #ifdef DEBUG
            printf("\nStar Rating: \033[1;33m%.2f\033[0m\nMax Combo: \033[1;32m%d\033[0m\nAim: \033[1;36m%.2f\033[0m\nSpeed: \033[1;31m%.2f\033[0m\nSpeed Note Count: \033[0m%.2f\nFlashlight: \033[1;34m%.2f\033[0m\nSlider Factor: %.2f\nAR: \033[0;35m%.2f\033[0m\nOD: \033[1;35m%.2f\033[0m\n\n", attributes->stars, attributes->maxcombo, attributes->aim, attributes->speed, attributes->speednotecount, attributes->flashlight, attributes->sliderfactor, attributes->ar, attributes->od);
            printf("Count Circles: %d | Count Sliders: %d | Count Spinners %d\n\n", attributes->countcircle, attributes->countsliders, attributes->countspinners);
        #endif

        /* Calc beatmaps PP with current osu performance point formula */
        float pp = calcTotal(data, attributes, mods);

        #ifdef DEBUG
            printf("PP:\033[1;31m %.2f\033[0m\n\n", pp);
        #endif

        /* Write to twitch chat here */
        char response[50] = {0};
        memset(response, '\0', 50);
        sprintf(response, "PRIVMSG #%s :PP->%.2f\r\n", channel, pp);
        send(fd, response, strlen(response), 0);
    }

    if(!strncmp("!s", command, 3) && !strncmp(OWNER, user, strlen(OWNER))){
        #ifdef DEBUG
            printf("\nKilling bot...\n");
        #endif
        return false;
    }

    if(!strncmp("!ms", command, 4)){
        double twitch_ms = ping_socket("irc.chat.twitch.tv");
        if((int)twitch_ms < 0){
            printf("irc.chat.twitch.tv has timed out!\n");
            return false;
        }
        double osu_ms = ping_socket("osu.ppy.sh");
        if((int)osu_ms < 0){
            printf("osu.ppy.sh has timed out!\n");
            return false;
        }

        char pres[255];
        memset(pres, '\0', 255);
        sprintf(pres, "PRIVMSG #%s :| ttv irc-> %.2fms || osu api-> %.2fms |\r\n", channel, twitch_ms, osu_ms);
        send(fd, pres, strlen(pres), 0);
    }
    return true;
}
