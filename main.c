#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include "headers/parse.h"
#include "headers/computepp.h"
#include "headers/twitch.h"
#include "headers/apiv2.h"

/*
Osu PP Calculator bot for twitch.tv

Only calculates %100 Acc; full combo scores for now
Will add custom Acc and Combo values in the near future

to do:
- Add threading
- Finish mods
- Acc calculation
- Combo calculation
*/

#define VERSION "1.2.0"

#define CHANNEL "znods"
#define BOTNAME "znodss"
#define OAUTH_KEY "your oauth key"
/*
Get key from here:
https://chatterino.com/client_login
*/

#define BUFSIZE 15000

float calcTotal(struct beatmap_data *, struct beatmap *, int);

int main(){
    struct beatmap *attributes = (struct beatmap *)malloc(sizeof(beatmap_t));
    struct beatmap_data *data = (struct beatmap_data *)malloc(sizeof(beatmap_data_t));
    char *buffer = (char *)malloc(BUFSIZE * sizeof(char));
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

    while(running){
        twitch_chat = (char *)malloc(BUFSIZE * sizeof(char));
        memset(twitch_chat, '\0', BUFSIZE);
        read(fd, twitch_chat, BUFSIZE);
        /* Parse Twitch Chat */
        parse_chat(twitch_chat, buffer);
        /* Check for Commands */
        running = commands(fd, buffer, CHANNEL, attributes, data);
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
    close(fd);
    #ifdef DEBUG
        puts("\n\nEXITED GRACEFULLY!\n\n");
    #endif
    return 0;
}

float calcTotal(struct beatmap_data *data, struct beatmap *attributes, int mods){
    // only calculating 100% scores for NOW
    data->num300 = attributes->countcircle + attributes->countsliders + attributes->countspinners;
    data->num100 = 0;
    data->num50 = 0;
    data->numMiss = 0;

    data->maxcombo = attributes->maxcombo;
    data->numsliders = attributes->countsliders;
    data->aim = attributes->aim;
    data->sliderfactor = attributes->sliderfactor;
    data->ar = attributes->ar;
    data->od = attributes->od;
    data->speed = attributes->speed;
    data->speednotecount = attributes->speednotecount;
    data->flashlight = attributes->flashlight;

    data->totalhitcircles = attributes->countcircle + attributes->countsliders + attributes->countspinners;

    compute_effective_misscount(data);
    computeAimValue(data, mods);
    computeSpeedValue(data, mods);
    computeAccuracyValue(data, mods);
    computeFlashLight(data, mods);

    return computeTotalValue(mods);
}
