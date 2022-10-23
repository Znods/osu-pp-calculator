#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include "headers/parse.h"
#include "headers/computepp.h"
#include "headers/twitch.h"

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

#define VERSION "1.0.0"

#define CHANNEL "your_channel"
#define BOTNAME "your_botname"
#define OAUTH_KEY "oauth key"
/*
Get key from here:
https://chatterino.com/client_login
*/

#define BUFSIZE 15000

float calcTotal(struct beatmap_data *, struct beatmap *);

int main(){
    struct beatmap *attributes = (struct beatmap *)malloc(sizeof(beatmap_t));
    struct beatmap_data *data = (struct beatmap_data *)malloc(sizeof(beatmap_data_t));
    char *twitch_chat = (char *)malloc(BUFSIZE * sizeof(char));
    char *buffer = (char *)malloc(BUFSIZE * sizeof(char));
    bool running = true;

    /* Connect to twitch chat */
    int fd = twitch_socket();
    twitch_login(fd, CHANNEL, BOTNAME, OAUTH_KEY);

    while(running){
        memset(twitch_chat, '\0', BUFSIZE);
        read(fd, twitch_chat, BUFSIZE);
        /* Parse Twitch Chat */
        parse_chat(twitch_chat, buffer);
        /* Check for Commands */
        commands(fd, buffer, CHANNEL, attributes, data);
        /* Write Twitch Chat to Console */
        #ifdef DEBUG
            write(1, buffer, strlen(buffer));
        #endif
        /* Check for PING */
        ping_check(fd, twitch_chat);
        usleep(1000);
    }

    free(attributes);
    free(data);
    free(twitch_chat);
    free(buffer);

    return 0;
}

float calcTotal(struct beatmap_data *data, struct beatmap *attributes){
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
    computeAimValue(data);
    computeSpeedValue(data);
    computeAccuracyValue(data);
    computeFlashLight(data);

    return computeTotalValue();
}
