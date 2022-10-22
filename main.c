#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "headers/parse.h"
#include "headers/computepp.h"

int osu_apiv2(struct beatmap *);
float calcTotal(struct beatmap_data *data, struct beatmap *attributes);

int main(){
    struct beatmap *attributes = (struct beatmap *)malloc(sizeof(beatmap_t));
    struct beatmap_data *data = (struct beatmap_data *)malloc(sizeof(beatmap_data_t));
    
    /* Get information about beatmap from osu apiv2 */
    int ret = osu_apiv2(attributes);
    if(ret < 0){
        printf("\033[0;35mosu api failed!?\033[0m\n");
        exit(1);
    }

    #ifdef DEBUG
        printf("\nStar Rating: \033[1;33m%.2f\033[0m\nMax Combo: \033[1;32m%d\033[0m\nAim: \033[1;36m%.2f\033[0m\nSpeed: \033[1;31m%.2f\033[0m\nSpeed Note Count: \033[0m%.2f\nFlashlight: \033[1;34m%.2f\033[0m\nSlider Factor: %.2f\nAR: \033[0;35m%.2f\033[0m\nOD: \033[1;35m%.2f\033[0m\n\n", attributes->stars, attributes->maxcombo, attributes->aim, attributes->speed, attributes->speednotecount, attributes->flashlight, attributes->sliderfactor, attributes->ar, attributes->od);
        printf("Count Circles: %d\nCount Sliders: %d\nCount Spinners %d\n\n", attributes->countcircle, attributes->countsliders, attributes->countspinners);
    #endif

    /* Calc beatmaps PP with current osu performance point formula */
    float pp = calcTotal(data, attributes);

    #ifdef DEBUG
        printf("PP:\033[1;31m %.2f\033[0m\n\n", pp);
    #endif

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
