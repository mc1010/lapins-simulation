#include <stdio.h>
#include <string.h>
#include "simulation.h"
#include "Population.h"
#include "Random.h"
#include "Survival.h"
#include "params.h"

long long capacityK = 0;   /* 0 = pas de limite */
long long deathAgeHist[2][MAX_AGE_MONTHS + 1];

void resetDeathStats(void)
{
    memset(deathAgeHist, 0, sizeof(deathAgeHist));
}

void agePopulation(Population *pop)
{
    int i;
    for (i = 0; i < pop->count; i++)
        pop->rabbits[i].age++;
}

void handleMortality(Population *pop, Monthly_Stats *stats, int *matureMales)
{
    int i;
    double extra = 1.0;
    *matureMales = 0;

    /* Si N > K, chaque lapin survit en plus avec la probabilite K/N.
       Calcule une seule fois par mois, avant la boucle. */
    if (capacityK > 0 && pop->count > capacityK)
        extra = (double)capacityK / (double)pop->count;

    /* On parcourt a l'envers : removeRabbit ramene le dernier lapin
       a la place du mort, et celui-la a deja ete traite. */
    for (i = pop->count - 1; i >= 0; i--)
    {
        Rabbit *r = &pop->rabbits[i];
        int adulte = (r->age >= r->maturity_age);

        if (randChance(getMonthlySurvival(r->age, adulte) * extra))
        {
            if (r->sex == Male && adulte)
                (*matureMales)++;
        }
        else
        {
            if (r->age <= MAX_AGE_MONTHS)
                deathAgeHist[adulte ? 1 : 0][r->age]++;   /* avant removeRabbit */
            removeRabbit(pop, i);
            stats->deaths++;
        }
    }
}

int handleReproduction(Population *pop, Monthly_Stats *stats, int matureMales)
{
    int i, k, m, L, kits;
    int n = pop->count;

    for (i = 0; i < n; i++)
    {
        if (pop->rabbits[i].sex != Female) continue;
        if (pop->rabbits[i].age < pop->rabbits[i].maturity_age) continue;

        m = (pop->rabbits[i].age - pop->rabbits[i].maturity_age) % 12;
        if (m == 0)
        {
            L = drawLitterCount();
            pop->rabbits[i].litters_year = (uint8_t)L;
            pop->rabbits[i].litter_mask  = drawLitterMask(L);
        }
        if (matureMales > 0 && ((pop->rabbits[i].litter_mask >> m) & 1))
        {
            kits = randInt(KITS_MIN, KITS_MAX);
            for (k = 0; k < kits; k++)
                if (!addRabbit(pop, createRabbit(0))) return 0;
            stats->births += kits;
            stats->litters++;
        }
    }
    return 1;
}

int writeAgeDistribution(const Population *pop, const char *filename)
{
    long long males[MAX_AGE_MONTHS + 1], females[MAX_AGE_MONTHS + 1];
    int i, a;
    FILE *f;

    memset(males, 0, sizeof(males));
    memset(females, 0, sizeof(females));
    for (i = 0; i < pop->count; i++)
    {
        a = pop->rabbits[i].age;
        if (a > MAX_AGE_MONTHS) a = MAX_AGE_MONTHS;
        if (pop->rabbits[i].sex == Male) males[a]++;
        else                             females[a]++;
    }

    f = fopen(filename, "w");
    if (f == NULL) return 0;
    fprintf(f, "age,males,females\n");
    for (a = 0; a <= MAX_AGE_MONTHS; a++)
        fprintf(f, "%d,%lld,%lld\n", a, males[a], females[a]);
    fclose(f);
    return 1;
}

int writeDeathAges(const char *filename)
{
    int a;
    FILE *f = fopen(filename, "w");
    if (f == NULL) return 0;
    fprintf(f, "age,deces_jeunes,deces_adultes\n");
    for (a = 0; a <= MAX_AGE_MONTHS; a++)
        fprintf(f, "%d,%lld,%lld\n", a, deathAgeHist[0][a], deathAgeHist[1][a]);
    fclose(f);
    return 1;
}