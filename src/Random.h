#ifndef RANDOM_H
#define RANDOM_H

#include "structures.h"

// Initialise le générateur (Mersenne Twister) avec une graine
void initRandom(unsigned long seed);

// Entier uniforme dans [min, max] (bornes incluses). 
int randInt(int min, int max);

// Renvoie 1 avec la probabilité donnée (entre 0 et 1), sinon 0. 
int randChance(double probability);

// Renvoie Female avec la probabilité P_FEMALE, sinon Male. 
Sex randomSex(void);

#endif