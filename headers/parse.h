#pragma once

#include <json-c/json.h>

struct json_object *parsed_json;
struct json_object *parsed_json2;

struct json_object *attribute;

/* Objects inside attributes */

struct json_object *star_rating;
struct json_object *max_combo;
struct json_object *aim_difficulty;
struct json_object *speed_difficulty;
struct json_object *speed_note_count;
struct json_object *flashlight_difficulty;
struct json_object *slider_factor;
struct json_object *approach_rate;
struct json_object *overall_difficulty;

// additional info for beatmap

struct json_object *count_circles;
struct json_object *count_sliders;
struct json_object *count_spinners;

struct beatmap{
    int num50;
    int num100;
    int num300;
    int numMiss;
    float stars;
    int maxcombo;
    float aim;
    float speed;
    float speednotecount;
    float flashlight;
    float sliderfactor;
    float ar;
    float od;
    int countcircle;
    int countsliders;
    int countspinners;
    int totalHitCircles;
}beatmap_t;

void parse_attributes(char *, struct beatmap *);
void parse_additional_info(char *, struct beatmap *);
int osu_apiv2(struct beatmap *, int, int);
void parse_chat(char *, char *);
