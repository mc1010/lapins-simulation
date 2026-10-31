#include "Random.h"
#include "params.h"
#include "mt19937ar.h"

void initRandom(unsigned long seed)
{
    init_genrand(seed);
}

static double uniform01(void)
{
    return genrand_real2();
}

int randInt(int min, int max)
{
    if (max < min)
    {
        int tmp = min;
        min = max;
        max = tmp;
    }
    return min + (int)(uniform01() * (max - min + 1));
}

int randChance(double probability)
{
    if (uniform01() < probability)
        return 1;
    else
        return 0;
}

Sex randomSex(void)
{
    if (randChance(P_FEMALE))
        return Female;
    else
        return Male;
}