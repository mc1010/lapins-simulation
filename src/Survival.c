#include <math.h>
#include "Survival.h"
#include "params.h"

static double survivalTable[2][MAX_AGE_MONTHS + 1];

static double annualSurvival(int age, int adult)
{
    int year = age / 12;
    double s;

    if (age >= MAX_AGE_MONTHS) return 0.0;
    if (!adult) return SURVIVAL_YOUNG;

    s = SURVIVAL_ADULT;
    if (year >= OLD_AGE_START)
        s = SURVIVAL_ADULT - OLD_AGE_STEP * (year - OLD_AGE_START + 1);
    if (s < 0.0) s = 0.0;
    return s;
}

void initSurvivalTable(void)
{
    int age, stage;
    for (stage = 0; stage < 2; stage++)
        for (age = 0; age <= MAX_AGE_MONTHS; age++)
            survivalTable[stage][age] = pow(annualSurvival(age, stage), 1.0 / 12.0);
}

double getMonthlySurvival(int age, int adult)
{
    if (age > MAX_AGE_MONTHS) age = MAX_AGE_MONTHS;
    return survivalTable[adult ? 1 : 0][age];
}