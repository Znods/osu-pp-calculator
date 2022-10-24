#pragma once

struct beatmap_data{
    int num300;
    int num100;
    int num50;
    int numMiss;
    int maxcombo;
    int numsliders;
    float aim;
    float sliderfactor;
    float ar;
    float od;
    float speed;
    float speednotecount;
    int totalhitcircles;
    float flashlight;
}beatmap_data_t;

float aimValue;
float speedValue;
float accuracyValue;
float flashlightValue;
float effectiveMissCount;
float totalValue;

enum scoreVersion {SV1, SV2};

float accuracy(struct beatmap_data *);
int total_hits(struct beatmap_data *);
int total_successful_hits(struct beatmap_data *);
void compute_effective_misscount(struct beatmap_data *);
void computeAimValue(struct beatmap_data *, int);
void computeSpeedValue(struct beatmap_data *, int);
void computeAccuracyValue(struct beatmap_data *, int);
void computeFlashLight(struct beatmap_data *, int);
float getComboScalingFactor(struct beatmap_data *);
float computeTotalValue(int);
