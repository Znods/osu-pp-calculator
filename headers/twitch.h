#pragma once
#include <stdbool.h>

int twitch_socket();
int twitch_login(int, char *, char *, char *);
int ping_check(int, char *);
bool commands(int, char *, char *, struct beatmap *, struct beatmap_data *, char *);
