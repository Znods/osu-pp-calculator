#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <pthread.h>

#include "headers/parse.h"
#include "headers/computepp.h"
#include "headers/twitch.h"

/*
Osu PP Calculator bot for twitch.tv

Only calculates %100 Acc; full combo scores for now
Will add custom Acc and Combo values in the near future

*/

#define VERSION "1.3.0"

#define CHANNEL "znods"
#define BOTNAME "znodss"
#define OAUTH_KEY "your oauth key"

/* Get key from here: https://chatterino.com/client_login */

const int client_id = 1337;
const char *client_secret = "your client secret";

/*  Find client id and client secret from your osu settings page!!!!!
                https://osu.ppy.sh/home/account/edit                    */

#define BUFSIZE 15000

int main(){
    struct beatmap *attributes = (struct beatmap *)malloc(1 * sizeof(beatmap_t));
    struct beatmap_data *data = (struct beatmap_data *)malloc(1 * sizeof(beatmap_data_t));
    char *buffer = (char *)malloc(BUFSIZE * sizeof(char));
    char *osutoken = (char *)malloc(2024 * sizeof(char));
    char *twitch_chat = '\0';
    bool running = true;

    /* Connect to twitch chat */
    #ifdef DEBUG
        write(1, "\033[0;35mSetting up socket...\n\033[0m", 33);
    #endif
    int fd = twitch_socket();
    #ifdef DEBUG
        write(1, "\033[0;35mLogging into twitch irc...\n\033[0m", 39);
    #endif
    twitch_login(fd, CHANNEL, BOTNAME, OAUTH_KEY);

    #ifdef DEBUG
        printf("\t\t\033[1;35mPP Bot started! v%s\n\033[0m", VERSION);
    #endif

    int ret = get_token(osutoken);
    if(ret < 0){
        printf("\n\n\033[1;31mFailed grabbing osu api token!\nCheck client_id & client_secret in apiv2.c!\033[0m\n\n\n");
        free(attributes);
        free(data);
        free(buffer);
        free(osutoken);
        close(fd);
        return EXIT_FAILURE;
    }
    #ifdef DEBUG
        write(1, "\n\t\t\033[1;31mGot osu apiv2 token!\033[0\n", 35);
    #endif

    while(running){
        twitch_chat = (char *)malloc(BUFSIZE * sizeof(char));
        memset(twitch_chat, '\0', BUFSIZE);
        read(fd, twitch_chat, BUFSIZE);
        /* Parse Twitch Chat */
        parse_chat(twitch_chat, buffer);
        /* Check for Commands */
        running = commands(fd, buffer, CHANNEL, attributes, data, osutoken);
        /* Write Twitch Chat to Console */
        #ifdef DEBUG
            write(1, buffer, strlen(buffer));
        #endif
        /* Check for PING */
        ping_check(fd, twitch_chat);
        free(twitch_chat);
        usleep(1000);
    }

    #ifdef DEBUG
        puts("\nCleaning up...\n");
    #endif
    free(attributes);
    free(data);
    free(buffer);
    free(osutoken);
    close(fd);
    #ifdef DEBUG
        puts("EXITED GRACEFULLY!\n");
    #endif
    return 0;
}
