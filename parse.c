#include <json-c/json.h>
#ifdef DEBUG
    #include <stdio.h>
#endif
#include "headers/parse.h"

void parse_attributes(char *buffer, struct beatmap *attributes){
    parsed_json = json_tokener_parse(buffer);

    json_object_object_get_ex(parsed_json, "attributes", &attribute);

    json_object_object_get_ex(attribute, "star_rating", &star_rating);
    json_object_object_get_ex(attribute, "max_combo", &max_combo);
    json_object_object_get_ex(attribute, "aim_difficulty", &aim_difficulty);
    json_object_object_get_ex(attribute, "speed_difficulty", &speed_difficulty);
    json_object_object_get_ex(attribute, "speed_note_count", &speed_note_count);
    json_object_object_get_ex(attribute, "flashlight_difficulty", &flashlight_difficulty);
    json_object_object_get_ex(attribute, "slider_factor", &slider_factor);
    json_object_object_get_ex(attribute, "approach_rate", &approach_rate);
    json_object_object_get_ex(attribute, "overall_difficulty", &overall_difficulty);

    attributes->stars = json_object_get_double(star_rating);
    attributes->maxcombo = json_object_get_int(max_combo);
    attributes->aim = json_object_get_double(aim_difficulty);
    attributes->speed = json_object_get_double(speed_difficulty);
    attributes->speednotecount = json_object_get_double(speed_note_count);
    attributes->flashlight = json_object_get_double(flashlight_difficulty);
    attributes->sliderfactor = json_object_get_double(slider_factor);
    attributes->ar = json_object_get_double(approach_rate);
    attributes->od = json_object_get_double(overall_difficulty);
}

void parse_additional_info(char *buffer, struct beatmap *attributes){
    parsed_json2 = json_tokener_parse(buffer);

    json_object_object_get_ex(parsed_json2, "count_circles", &count_circles);
    json_object_object_get_ex(parsed_json2, "count_sliders", &count_sliders);
    json_object_object_get_ex(parsed_json2, "count_spinners", &count_spinners);

    attributes->countcircle = json_object_get_int(count_circles);
    attributes->countsliders = json_object_get_int(count_sliders);
    attributes->countspinners = json_object_get_int(count_spinners);
}