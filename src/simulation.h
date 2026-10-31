#ifndef SIMULATION_H
#define SIMULATION_H

#include "structures.h"
#include "params.h"

/* Capacite maximale K (en lapins). 0 = pas de limite (modele libre).
   Si la population depasse K, chaque lapin survit avec la probabilite K/N. */
extern long long capacityK;

/* Nombre de morts par age (en mois) : [0] = jeunes, [1] = adultes. */
extern long long deathAgeHist[2][MAX_AGE_MONTHS + 1];

/* Remet a zero le compteur des ages au deces (a appeler avant la simulation). */
void resetDeathStats(void);

/* Ajoute 1 mois a l'age de chaque lapin. */
void agePopulation(Population *pop);

/* Tire la survie de chaque lapin, retire les morts, les compte dans stats->deaths,
   enregistre leur age, et met dans *matureMales le nombre de males matures vivants. */
void handleMortality(Population *pop, Monthly_Stats *stats, int *matureMales);

/* Naissances du mois. Une femelle mature met bas si son masque l'indique
   ET si matureMales > 0. */
int handleReproduction(Population *pop, Monthly_Stats *stats, int matureMales);

/* Ecrit la repartition des ages de la population (age, males, femelles). */
int writeAgeDistribution(const Population *pop, const char *filename);

/* Ecrit les ages au deces (age, deces_jeunes, deces_adultes). */
int writeDeathAges(const char *filename);

#endif