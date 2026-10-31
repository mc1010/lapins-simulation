/* ======================================================================
   lapins.c : programme unique du Lab 4a (simulation de population de lapins)

   Trois commandes :

     lapins run   [mois] [graine] [couples] [plafond_millions]
        Un run detaille. Ecrit run_c<couples>_s<graine>.csv
        + ages_final.csv (repartition des ages a la fin)
        + ages_deces.csv (age des lapins au moment de leur deces)

     lapins limit [couples] [graine] [plafond_millions]
        Fait grossir la population jusqu'au plafond (ou jusqu'a la fin de la
        memoire) pour trouver la limite du programme. Mesure le temps et la
        memoire a chaque mois. Ecrit limite_c<couples>_s<graine>.csv

     lapins exp   [reps] [mois] [plafond_millions] [couples ...]
        Plusieurs conditions initiales, plusieurs replications chacune.
        Ecrit exp_runs.csv, exp_resume.csv et exp_courbes.csv

   Les fichiers CSV sont ensuite traites par analyse.py (graphiques).
   ====================================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "structures.h"
#include "params.h"
#include "Random.h"
#include "Population.h"
#include "Survival.h"
#include "simulation.h"


#define HARD_LIMIT 800000000LL

typedef enum { STOP_HORIZON, STOP_EXTINCTION, STOP_CEILING, STOP_MEMORY } StopReason;

typedef struct {
    StopReason reason;
    int        lastMonth;      
    int        extinctMonth;  
    int        ceilingMonth;   
    long long  finalSize;
    long long  finalFemales;
    long long  maxSize;
    long long  totalBirths;
    long long  totalDeaths;
    double     seconds;
    double     memoryMB;       
} RunResult;

static const char *reasonText(StopReason r)
{
    switch (r)
    {
        case STOP_HORIZON:    return "horizon";
        case STOP_EXTINCTION: return "extinction";
        case STOP_CEILING:    return "plafond";
        default:              return "memoire";
    }
}

static void countSexes(const Population *pop, long long *males, long long *females)
{
    int i;
    *males = 0;
    *females = 0;
    for (i = 0; i < pop->count; i++)
    {
        if (pop->rabbits[i].sex == Male) (*males)++;
        else                             (*females)++;
    }
}

static RunResult simulate(unsigned long seed, int couples, int months, long long ceiling,
                          long long *traj, FILE *csv, int verbose, int dumpAges)
{
    Population pop;
    Monthly_Stats stats;
    RunResult res;
    int month, m, matureMales = 0;
    long long males, females;
    clock_t start = clock();

    memset(&res, 0, sizeof res);
    res.reason = STOP_HORIZON;
    res.extinctMonth = -1;
    res.ceilingMonth = -1;
    if (ceiling > HARD_LIMIT) ceiling = HARD_LIMIT;

    initRandom(seed);
    resetDeathStats();  
    if (!initPopulation(&pop, 1024) || !addInitialCouples(&pop, couples))
    {
        freePopulation(&pop);
        res.reason = STOP_MEMORY;
        return res;
    }

    res.maxSize = pop.count;
    if (traj) traj[0] = pop.count;
    if (csv)
    {
        fprintf(csv, "month,total,males,females,mature_males,births,deaths,litters,seconds,memory_mb\n");
        countSexes(&pop, &males, &females);
        fprintf(csv, "0,%d,%lld,%lld,%d,0,0,0,0.00,%.2f\n",
                pop.count, males, females, couples,
                (double)pop.capacity * sizeof(Rabbit) / 1e6);
    }

    for (month = 1; month <= months; month++)
    {
        stats.births = 0;
        stats.deaths = 0;
        stats.litters = 0;

        agePopulation(&pop);
        handleMortality(&pop, &stats, &matureMales);
        if (!handleReproduction(&pop, &stats, matureMales))
        {
            res.reason = STOP_MEMORY;
            break;
        }

        res.lastMonth = month;
        res.totalBirths += stats.births;
        res.totalDeaths += stats.deaths;
        if (pop.count > res.maxSize) res.maxSize = pop.count;
        if (traj) traj[month] = pop.count;

        if (csv)
        {
            countSexes(&pop, &males, &females);
            fprintf(csv, "%d,%d,%lld,%lld,%d,%lld,%lld,%lld,%.2f,%.2f\n",
                    month, pop.count, males, females, matureMales,
                    stats.births, stats.deaths, stats.litters,
                    (double)(clock() - start) / CLOCKS_PER_SEC,
                    (double)pop.capacity * sizeof(Rabbit) / 1e6);
            fflush(csv);   
        }
        if (verbose && month % 6 == 0)
            printf("mois %4d : %12d lapins | %8.1f s | %7.0f Mo\n", month, pop.count,
                   (double)(clock() - start) / CLOCKS_PER_SEC,
                   (double)pop.capacity * sizeof(Rabbit) / 1e6);

        if (pop.count == 0)
        {
            res.reason = STOP_EXTINCTION;
            res.extinctMonth = month;
            break;
        }
        if (pop.count >= ceiling)
        {
            res.reason = STOP_CEILING;
            res.ceilingMonth = month;
            break;
        }
    }
    if (traj)
        for (m = res.lastMonth + 1; m <= months; m++)
            traj[m] = (res.reason == STOP_EXTINCTION) ? 0 : -1;
    if (dumpAges)
    {
        writeAgeDistribution(&pop, "ages_final.csv");
        writeDeathAges("ages_deces.csv");
    }

    countSexes(&pop, &males, &females);
    res.finalSize = pop.count;
    res.finalFemales = females;
    res.memoryMB = (double)pop.capacity * sizeof(Rabbit) / 1e6;
    res.seconds = (double)(clock() - start) / CLOCKS_PER_SEC;
    freePopulation(&pop);
    return res;
}

/* ---------------------------- Statistiques --------------------------- */

static int comparer(const void *a, const void *b)
{
    double x = *(const double *)a, y = *(const double *)b;
    return (x > y) - (x < y);
}


static double quantile(const double *t, int n, double q)
{
    double pos, f;
    int i;
    if (n <= 0) return 0.0;
    pos = q * (n - 1);
    i = (int)pos;
    f = pos - i;
    if (i + 1 >= n) return t[n - 1];
    return t[i] * (1.0 - f) + t[i + 1] * f;
}


static void wilson(int k, int n, double *bas, double *haut)
{
    double z = 1.96, p = (double)k / n;
    double den = 1.0 + z * z / n;
    double centre = (p + z * z / (2.0 * n)) / den;
    double marge = z * sqrt(p * (1.0 - p) / n + z * z / (4.0 * n * (double)n)) / den;
    *bas = centre - marge;
    *haut = centre + marge;
}

static int cmdRun(const char *prefix, int months, unsigned long seed, int couples,
                  long long ceiling, int dumpAges)
{
    char name[96];
    FILE *csv;
    RunResult r;

    sprintf(name, "%s_c%d_s%lu.csv", prefix, couples, seed);
    csv = fopen(name, "w");
    if (csv == NULL)
    {
        printf("Impossible de creer %s\n", name);
        return 1;
    }
    printf("Simulation : %d couple(s), graine %lu, %d mois max, plafond %lld lapins\n\n",
           couples, seed, months, ceiling > HARD_LIMIT ? HARD_LIMIT : ceiling);

    r = simulate(seed, couples, months, ceiling, NULL, csv, 1, dumpAges);
    fclose(csv);

    printf("\n--- Resume ---\n");
    printf("Arret            : %s apres %d mois\n", reasonText(r.reason), r.lastMonth);
    printf("Population finale: %lld lapins (%lld femelles)\n", r.finalSize, r.finalFemales);
    printf("Maximum atteint  : %lld lapins\n", r.maxSize);
    printf("Naissances / morts totales : %lld / %lld\n", r.totalBirths, r.totalDeaths);
    printf("Temps de calcul  : %.2f s\n", r.seconds);
    printf("Memoire du tableau : %.0f Mo\n", r.memoryMB);
    printf("Fichier ecrit    : %s\n", name);
    if (dumpAges)
        printf("Fichiers ecrits  : ages_final.csv, ages_deces.csv\n");
    return 0;
}
static int cmdExp(int reps, int months, long long ceiling, int nbCond, const int *conds)
{
    FILE *fruns, *fres, *fcur;
    long long *traj;
    double *vals, *surv;
    int c, r, m;

    traj = malloc(sizeof(long long) * (size_t)reps * (size_t)(months + 1));
    vals = malloc(sizeof(double) * (size_t)reps);
    surv = malloc(sizeof(double) * (size_t)reps);
    fruns = fopen("exp_runs.csv", "w");
    fres  = fopen("exp_resume.csv", "w");
    fcur  = fopen("exp_courbes.csv", "w");
    if (!traj || !vals || !surv || !fruns || !fres || !fcur)
    {
        printf("Erreur : memoire ou creation des fichiers CSV impossible\n");
        return 1;
    }

    fprintf(fruns, "couples,graine,arret,mois_extinction,mois_plafond,dernier_mois,effectif_final,femelles_final,effectif_max,secondes\n");
    fprintf(fres,  "couples,reps,extinctions,extinction_pct,wilson_bas_pct,wilson_haut_pct,mois_extinction_moyen,"
                   "survivants,mediane,q1,q3,min,max,interrompus,secondes_total\n");
    fprintf(fcur,  "couples,mois,n_valides,n_vivants,frac_extinction,q1,mediane,q3\n");

    printf("%d replications, %d mois, plafond %lld lapins, %d condition(s)\n\n", reps, months, ceiling, nbCond);

    for (c = 0; c < nbCond; c++)
    {
        int couples = conds[c];
        int extinctions = 0, interrupted = 0, nSurv = 0;
        double sumExtMonth = 0.0, totalSeconds = 0.0, lo, hi;

        for (r = 0; r < reps; r++)
        {
            unsigned long seed = BASE_SEED + 100000UL * (unsigned long)couples + (unsigned long)r;
            RunResult res = simulate(seed, couples, months, ceiling, traj + (size_t)r * (months + 1), NULL, 0, 0);

            totalSeconds += res.seconds;
            if (res.reason == STOP_EXTINCTION) { extinctions++; sumExtMonth += res.extinctMonth; }
            else if (res.reason == STOP_HORIZON) surv[nSurv++] = (double)res.finalSize;
            else interrupted++;

            fprintf(fruns, "%d,%lu,%s,%d,%d,%d,%lld,%lld,%lld,%.2f\n", couples, seed,
                    reasonText(res.reason), res.extinctMonth, res.ceilingMonth, res.lastMonth,
                    res.finalSize, res.finalFemales, res.maxSize, res.seconds);
            printf("\r  %2d couple(s) : replication %d/%d", couples, r + 1, reps);
            fflush(stdout);
        }

      
        wilson(extinctions, reps, &lo, &hi);
        qsort(surv, nSurv, sizeof(double), comparer);
        fprintf(fres, "%d,%d,%d,%.2f,%.2f,%.2f,%.2f,%d,%.0f,%.0f,%.0f,%.0f,%.0f,%d,%.2f\n",
                couples, reps, extinctions, 100.0 * extinctions / reps, 100.0 * lo, 100.0 * hi,
                extinctions ? sumExtMonth / extinctions : 0.0,
                nSurv, quantile(surv, nSurv, 0.5), quantile(surv, nSurv, 0.25), quantile(surv, nSurv, 0.75),
                nSurv ? surv[0] : 0.0, nSurv ? surv[nSurv - 1] : 0.0, interrupted, totalSeconds);

     
        for (m = 0; m <= months; m++)
        {
            int nValid = 0, nAlive = 0, nExt = 0;
            for (r = 0; r < reps; r++)
            {
                long long v = traj[(size_t)r * (months + 1) + m];
                if (v < 0) continue;            
                nValid++;
                if (v == 0) nExt++;
                else vals[nAlive++] = (double)v;
            }
            qsort(vals, nAlive, sizeof(double), comparer);
            fprintf(fcur, "%d,%d,%d,%d,%.4f,%.0f,%.0f,%.0f\n", couples, m, nValid, nAlive,
                    nValid ? (double)nExt / nValid : 0.0,
                    quantile(vals, nAlive, 0.25), quantile(vals, nAlive, 0.5), quantile(vals, nAlive, 0.75));
        }

        printf("\r=== %d couple(s) de depart (%d lapins) ===                \n", couples, 2 * couples);
        printf("extinctions : %d / %d = %.1f %%   (IC 95 %% de Wilson : %.1f - %.1f %%)\n",
               extinctions, reps, 100.0 * extinctions / reps, 100.0 * lo, 100.0 * hi);
        if (extinctions > 0)
            printf("mois moyen d'extinction : %.1f\n", sumExtMonth / extinctions);
        if (nSurv > 0)
            printf("effectif final de %d survivants : mediane=%.0f  Q1=%.0f  Q3=%.0f  [min %.0f ; max %.0f]\n",
                   nSurv, quantile(surv, nSurv, 0.5), quantile(surv, nSurv, 0.25),
                   quantile(surv, nSurv, 0.75), surv[0], surv[nSurv - 1]);
        if (interrupted > 0)
            printf("(!) %d run(s) interrompus (plafond ou memoire) : exclus des effectifs finaux\n", interrupted);
        printf("temps de calcul de la condition : %.1f s\n\n", totalSeconds);
    }

    fclose(fruns); fclose(fres); fclose(fcur);
    free(traj); free(vals); free(surv);
    printf("Fichiers ecrits : exp_runs.csv, exp_resume.csv, exp_courbes.csv\n");
    return 0;
}


static void usage(void)
{
    printf("Usage :\n");
    printf("  lapins run   [mois] [graine] [couples] [plafond_millions]\n");
    printf("  lapins limit [couples] [graine] [plafond_millions]\n");
    printf("  lapins exp   [reps] [mois] [plafond_millions] [couples ...]\n\n");
    printf("Exemples :\n");
    printf("  lapins run 72 4 1\n");
    printf("  lapins limit 1 4 400\n");
    printf("  lapins exp 35 60 50 1 2 3 5 10\n");
}

int main(int argc, char *argv[])
{    
    {
        int i, j = 1;
        for (i = 1; i < argc; i++)
        {
            if (strncmp(argv[i], "K=", 2) == 0) capacityK = atoll(argv[i] + 2);
            else argv[j++] = argv[i];
        }
        argc = j;
    }
    initSurvivalTable();

    if (argc < 2) { usage(); return 0; }

    if (strcmp(argv[1], "run") == 0)
    {
        int months = argc > 2 ? atoi(argv[2]) : HORIZON_MONTHS;
        unsigned long seed = argc > 3 ? strtoul(argv[3], NULL, 10) : BASE_SEED;
        int couples = argc > 4 ? atoi(argv[4]) : INITIAL_COUPLES;
        double plafondM = argc > 5 ? atof(argv[5]) : 800.0;
        if (months < 1 || couples < 1) { usage(); return 1; }
        return cmdRun("run", months, seed, couples, (long long)(plafondM * 1e6), 1);
    }
    if (strcmp(argv[1], "limit") == 0)
    {
        int couples = argc > 2 ? atoi(argv[2]) : 1;
        unsigned long seed = argc > 3 ? strtoul(argv[3], NULL, 10) : BASE_SEED;
        double plafondM = argc > 4 ? atof(argv[4]) : 400.0;
        if (couples < 1) { usage(); return 1; }
        return cmdRun("limite", 100000, seed, couples, (long long)(plafondM * 1e6), 0);
    }
    if (strcmp(argv[1], "exp") == 0)
    {
        int reps = argc > 2 ? atoi(argv[2]) : 35;
        int months = argc > 3 ? atoi(argv[3]) : 60;
        double plafondM = argc > 4 ? atof(argv[4]) : 50.0;
        int defaults[5] = {1, 2, 3, 5, 10};
        int conds[64], nb = 0, i;

        for (i = 5; i < argc && nb < 64; i++) conds[nb++] = atoi(argv[i]);
        if (nb == 0) { memcpy(conds, defaults, sizeof defaults); nb = 5; }
        if (reps < 1 || months < 1) { usage(); return 1; }
        for (i = 0; i < nb; i++) if (conds[i] < 1) { usage(); return 1; }
        return cmdExp(reps, months, (long long)(plafondM * 1e6), nb, conds);
    }

    usage();
    return 1;
}