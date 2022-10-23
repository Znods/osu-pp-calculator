#ifdef DEBUG
    #include <stdio.h>
#endif

#include <math.h>

#include "headers/computepp.h"
#include "headers/tools.h"

float accuracy(struct beatmap_data *data){
    if(total_hits(data) != 0){
        return clamp((float)(data->num50 * 50 + data->num100 * 100 + data->num300 * 300) / (total_hits(data) * 300), 0.0f, 1.0f);
    }
    return 0;
}

int total_hits(struct beatmap_data *data){
    return data->num50 + data->num100 + data->num300 + data->numMiss;
}

int total_successful_hits(struct beatmap_data *data){
    return data->num50 + data->num100 + data->num300;
}

void compute_effective_misscount(struct beatmap_data *data){
    float comboBaseMissCount = 0.0f;
    if(data->numsliders > 0){
        float fullComboThreshold = data->maxcombo - 0.1f * data->numsliders;
        if(data->maxcombo < fullComboThreshold)
            comboBaseMissCount = fullComboThreshold / max(1, data->maxcombo);
    }

    // Clamp misscount to maximum amount of possible breaks
    comboBaseMissCount = min(comboBaseMissCount, (float)(data->num100 + data->num300 + data->numMiss));
    effectiveMissCount = max((float)data->numMiss, comboBaseMissCount);
}

float computeTotalValue(){
    enum mods mod;
    mod = NM;
    // Don't count scores made with supposedly unranked mods
    if(mod == RELAX || mod == RELAX2 || mod == AUTOPLAY){
        return 0.0f;
    }

    float multiplier = 1.14f;

    if(mod == NOFAIL)
        multiplier *= max(0.9f, 1.0f - 0.02f * effectiveMissCount);

    totalValue = pow(pow(aimValue, 1.1f) + pow(speedValue, 1.1f) + pow(accuracyValue, 1.1f) + pow(flashlightValue, 1.1), 1.0f / 1.1f) * multiplier;
    return totalValue;
}

void computeAimValue(struct beatmap_data *data){
    enum mods mod;
    mod = NM;
    aimValue = pow(5.0f * max(1.0f, data->aim / 0.0675f) - 4.0f, 3.0f) / 100000.0f;

    int numTotalHits = total_hits(data);

    float lengthBonus = 0.95f + 0.4f * min(1.0f, (float)numTotalHits / 2000.0f) + (numTotalHits > 2000 ? log10((float)numTotalHits / 2000.0f) * 0.5f : 0.0f);

    aimValue *= lengthBonus;

    // Penalize misses by assessing # of misses relative to the total # of objects. Default a 3% reduction for any # of misses.
    if(effectiveMissCount > 0)
        aimValue *= 0.97f * pow(1.0f - pow(effectiveMissCount / (float)numTotalHits, 0.775f), effectiveMissCount);

    aimValue *= getComboScalingFactor(data); ///?????? what is this????????????

    float approachRate = data->ar;
    float approachRateFactor = 0.0f;
    if(approachRate > 10.33f)
        approachRateFactor = 0.3f * (approachRate - 10.33f);
    else if(approachRate < 8.0f)
    approachRateFactor = 0.05f * (8.0f - approachRate);

    aimValue *= 1.0f + approachRateFactor * lengthBonus;

    // We want to give more reward for lower AR when it comes to aim and HD. This nerfs high AR and buffs lower AR.
    if(mod == HD)
        aimValue *= 1.0f + 0.04f * (12.0f - approachRate);

    // We assume 15% of sliders in a map are difficult since there's no way to tell from the performance calculator.
    float estimateDifficultSliders = data->numsliders * 0.15f;

    if(data->numsliders > 0){
        float mCombo = data->maxcombo;
        float estimateSliderEndsDropped = min(max(min((float)(data->num100 + data->num50 + data->numMiss), mCombo - data->maxcombo), 0.0f), estimateDifficultSliders);
        float sliderFactor = data->sliderfactor;
		float sliderNerfFactor = (1.0f - sliderFactor) * pow(1.0f - estimateSliderEndsDropped / estimateDifficultSliders, 3) + sliderFactor;
		aimValue *= sliderNerfFactor;
    }

    aimValue *= accuracy(data);
    // It is important to consider accuracy difficulty when scaling with accuracy.
	aimValue *= 0.98f + (pow(data->od, 2) / 2500);
}

void computeSpeedValue(struct beatmap_data *data){
    enum mods mod;
    mod = NM;
    speedValue = pow(5.0f * max(1.0f, data->speed / 0.0675f) - 4.0f, 3.0f) / 100000.0f;

	int numTotalHits = total_hits(data);

	float lengthBonus = 0.95f + 0.4f * min(1.0f, (float)(numTotalHits) / 2000.0f) + (numTotalHits > 2000 ? log10((float)(numTotalHits) / 2000.0f) * 0.5f : 0.0f);
	speedValue *= lengthBonus;

	// Penalize misses by assessing # of misses relative to the total # of objects. Default a 3% reduction for any # of misses.
	if (effectiveMissCount > 0)
		speedValue *= 0.97f * pow(1.0f - pow(effectiveMissCount / (float)numTotalHits, 0.775f), pow(effectiveMissCount, 0.875f));

	speedValue *= getComboScalingFactor(data); // wtf is this????????????????

	float approachRate = data->ar;
	float approachRateFactor = 0.0;
	if (approachRate > 10.33f)
		approachRateFactor = 0.3f * (approachRate - 10.33f);

	speedValue *= 1.0f + approachRateFactor * lengthBonus; // Buff for longer maps with high AR.

	// We want to give more reward for lower AR when it comes to speed and HD. This nerfs high AR and buffs lower AR.
	if (mod == HD)
		speedValue *= 1.0f + 0.04f * (12.0f - approachRate);

	// Calculate accuracy assuming the worst case scenario
	float relevantTotalDiff = (float)numTotalHits - data->speednotecount;
	float relevantCountGreat = max(0.0f, data->num300 - relevantTotalDiff);
	float relevantCountOk = max(0.0f, data->num100 - max(0.0f, relevantTotalDiff - data->num300));
	float relevantCountMeh = max(0.0f, data->num50 - max(0.0f, relevantTotalDiff - data->num300 - data->num100));
	float relevantAccuracy = data->speednotecount == 0.0f ? 0.0f : (relevantCountGreat * 6.0f + relevantCountOk * 2.0f + relevantCountMeh) / (data->speednotecount * 6.0f);

	// Scale the speed value with accuracy and OD.
	speedValue *= (0.95f + pow(data->od, 2) / 750) * pow((accuracy(data) + relevantAccuracy) / 2.0f, (14.5f - max(data->od, 8.0f)) / 2);

	// Scale the speed value with # of 50s to punish floattapping.
	speedValue *= pow(0.99f, data->num50 < numTotalHits / 500.0f ? 0.0f : data->num50 - numTotalHits / 500.0f);
}

void computeAccuracyValue(struct beatmap_data *data){
    enum scoreVersion scorev;
    enum mods mod;
    mod = NM;
    scorev = SV1;
    // This percentage only considers HitCircles of any value - in this part of the calculation we focus on hitting the timing hit window.
	float betterAccuracyPercentage;
	float numHitObjectsWithAccuracy;

	if (scorev == SV2)
	{
		numHitObjectsWithAccuracy = total_hits(data);
		betterAccuracyPercentage = accuracy(data);
	}
	// Either ScoreV1 or some unknown value. Let's default to previous behavior.
	else
	{
		numHitObjectsWithAccuracy = data->totalhitcircles;
		if (numHitObjectsWithAccuracy > 0)
			betterAccuracyPercentage = (float)((data->num300 - (total_hits(data) - numHitObjectsWithAccuracy)) * 6 + data->num100 * 2 + data->num50) / (numHitObjectsWithAccuracy * 6);
        else
			betterAccuracyPercentage = 0;

		// It is possible to reach a negative accuracy with this formula. Cap it at zero - zero points.
		if (betterAccuracyPercentage < 0)
			betterAccuracyPercentage = 0;
	}

    #ifdef DEBUG
        printf("Acc: %%%.2f\n", betterAccuracyPercentage * 100);
    #endif

	// Lots of arbitrary values from testing.
	// Considering to use derivation from perfect accuracy in a probabilistic manner - assume normal distribution.
	accuracyValue = pow(1.52163f, data->od) * pow(betterAccuracyPercentage, 24) * 2.83f;

	// Bonus for many hitcircles - it's harder to keep good accuracy up for longer.
	accuracyValue *= min(1.15f, (float)(pow(numHitObjectsWithAccuracy / 1000.0f, 0.3f)));

	if (mod == HD)
		accuracyValue *= 1.08f;

	if (mod == FL)
		accuracyValue *= 1.02f;
}

void computeFlashLight(struct beatmap_data *data){
    enum mods mod;
    mod = NM;
    flashlightValue = 0.0f;

	if (mod != FL)
		return;

	flashlightValue = pow(data->flashlight, 2.0f) * 25.0f;

	int numTotalHits = total_hits(data);

	// Penalize misses by assessing # of misses relative to the total # of objects. Default a 3% reduction for any # of misses.
	if (effectiveMissCount > 0)
		flashlightValue *= 0.97f * pow(1 - pow(effectiveMissCount / (float)(numTotalHits), 0.775f), pow(effectiveMissCount, 0.875f));

	flashlightValue *= getComboScalingFactor(data); // ???????!?????

	// Account for shorter maps having a higher ratio of 0 combo/100 combo flashlight radius.
	flashlightValue *= 0.7f + 0.1f * min(1.0f, (float)(numTotalHits) / 200.0f) + (numTotalHits > 200 ? 0.2f * min(1.0f, ((float)(numTotalHits) - 200) / 200.0f) : 0.0f);

	// Scale the flashlight value with accuracy _slightly_.
	flashlightValue *= 0.5f + accuracy(data) / 2.0f;
	// It is important to also consider accuracy difficulty when doing that.
	flashlightValue *= 0.98f + pow(data->od, 2.0f) / 2500.0f;
}

float getComboScalingFactor(struct beatmap_data *data){
    float mCombo = data->maxcombo;
	if (mCombo > 0)
		return min((float)pow(data->maxcombo, 0.8f) / pow(data->maxcombo, 0.8f), 1.0f);
	return 1.0f;
}
