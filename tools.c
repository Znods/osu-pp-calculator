#include <stdio.h>
#include <stdlib.h>

float clamp(float d, float min, float max) {
    const float t = d < min ? min : d;
    return t > max ? max : t;
}

float max(float a, float b){
    return (a < b) ? b : a;
}

float min(float a, float b){
    return (b < a) ? b : a;
}
