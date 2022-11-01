#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <pthread.h>
#include <signal.h>

#include "headers/parse.h"
#include "headers/computepp.h"
#include "headers/twitch.h"

/*
Osu PP Calculator bot for twitch.tv

Only calculates %100 Acc; full combo scores for now
Will add custom Acc and Combo values in the near future

*/

#define VERSION "1.4.0"

#define CHANNEL "znods"
#define BOTNAME "znodss"
#define OAUTH_KEY "your twitch oauth key"

/* Get oauth key from here: https://chatterino.com/client_login */

const int client_id = 1337; // your osu client id
const char *client_secret = "your osu client secret";

/*  Find client id and client secret from your osu settings page!!!!!
                https://osu.ppy.sh/home/account/edit                    */

#define BUFSIZE 15000

void cleanup(int, struct beatmap *, struct beatmap_data *, char *, char *, char *);
double ping_socket(char *);
void handler(int, siginfo_t *, void *);

int main(){
    struct beatmap *attributes = (struct beatmap *)malloc(1 * sizeof(beatmap_t));
    struct beatmap_data *data = (struct beatmap_data *)malloc(1 * sizeof(beatmap_data_t));
    char *buffer = (char *)malloc(BUFSIZE * sizeof(char));
    char *twitch_chat = (char *)malloc(BUFSIZE * sizeof(char));
    char *osutoken = (char *)malloc(2024 * sizeof(char));
    struct sigaction act;
    bool running = true;
    int fd = 0;

    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = &handler;

    if(sigaction(SIGINT, &act, NULL) < 0){
        perror("sigaction()\n");
        return EXIT_FAILURE;
    }

    #ifdef DEBUG
        write(1, "\033[0;35mSetting up socket...\n\033[0m", 33);
    #endif

    /* Setup Twitch Socket*/
    fd = twitch_socket();
    if(fd < 0){
        write(1, "\nERR in twitch_socket(), twitch.c\n", 35);
        return EXIT_FAILURE;
    }
    #ifdef DEBUG
        write(1, "\033[0;35mLogging into twitch irc...\n\033[0m", 39);
    #endif

    /* Login to twitch irc server */
    int ret = twitch_login(fd, CHANNEL, BOTNAME, OAUTH_KEY);
    if(ret == 2){
        printf("\nERR: in function twitch_login(), twitch.c\n");
        cleanup(fd, attributes, data, buffer, osutoken, twitch_chat);
        return EXIT_FAILURE;
    } else if(ret == -1){
        printf("Invalid oauth key or twitch name!\n\n");
        cleanup(fd, attributes, data, buffer, osutoken, twitch_chat);
        return EXIT_FAILURE;
    }

    #ifdef DEBUG
        printf("\n\t\033[36mirc.twitch.tv conntection established!\n");
    #endif

    /* Get token from osu apiv2 */
    ret = get_token(osutoken);
    if(ret < 0){
        printf("\n\n\033[1;31mFailed grabbing osu api token!\nCheck client_id & client_secret in apiv2.c!\033[0m\n\n\n");
        cleanup(fd, attributes, data, buffer, osutoken, twitch_chat);
        return EXIT_FAILURE;
    }
    #ifdef DEBUG
        write(1, "\t\t\033[0;35mGot osu apiv2 token!\033[0m\n\n", 36);
    #endif

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
    sleep(1);
    #ifdef DEBUG
        system("clear");
        printf("\t\033[1;35mtwitch\033[1;37m-> \033[1;35m%.2f\033[1;37mms,  \033[0;35m osu\033[1;37m-> \033[0;35m%.2f\033[1;37mms\033[0m\n", twitch_ms, osu_ms);
        printf("\n\t    \033[1;35mPP Bot started! v%s\n\033[0m\n", VERSION);
    #endif

    /* Main Loop */
    while(running){
        memset(twitch_chat, '\0', BUFSIZE);
        read(fd, twitch_chat, BUFSIZE);
        /* Parse Twitch Chat */
        parse_chat(twitch_chat, buffer);
        /* Check for Commands */
        running = commands(fd, buffer, CHANNEL, attributes, data, osutoken);
        /* Write Twitch Chat to Console */
        #ifdef DEBUG
            write(1, "\033[0m", 7);
            write(1, buffer, strlen(buffer));
        #endif
        /* Check for PING */
        ret = ping_check(fd, twitch_chat);
        if(ret < 0){
            write(1, "\n\npingCheck() function failed in twitch.c!\n", 44);
            running = false;
        }
        usleep(1000);
    }

    cleanup(fd, attributes, data, buffer, osutoken, twitch_chat);
    
    return EXIT_SUCCESS;
}

void handler(int sig, siginfo_t *info, void *x){
    if(sig == SIGINT){
        write(1, "\n\nProgram exit by root...\nGoodbye!\n", 36);
        exit(EXIT_SUCCESS);
    } else {
        printf("Received signal %d?\n", sig);
        exit(EXIT_FAILURE);
    }
}

void cleanup(int fd, struct beatmap *attributes, struct beatmap_data *data, char *buffer, char *osutoken, char *twitch_chat){
    #ifdef DEBUG
        puts("Cleaning up...");
    #endif
    free(attributes);
    free(data);
    free(buffer);
    free(osutoken);
    free(twitch_chat);
    close(fd);
    #ifdef DEBUG
        puts("Shutting down!");
    #endif
}
