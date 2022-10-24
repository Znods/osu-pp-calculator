#pragma once
#include <stdbool.h>

int twitch_socket();
void twitch_login(int, char *, char *, char *);
void ping_check(int, char *);
bool commands(int, char *, char *, struct beatmap *, struct beatmap_data *);