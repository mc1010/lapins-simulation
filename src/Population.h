#ifndef POPULATION_H
#define POPULATION_H

#include <stdint.h>
#include "structures.h"

int  initPopulation(Population *pop, int initialCapacity);
int  addRabbit(Population *pop, Rabbit r);
void removeRabbit(Population *pop, int index);
void freePopulation(Population *pop);

Rabbit   createRabbitOfSex(int age, Sex sex);
Rabbit   createRabbit(int age);
int      addInitialCouples(Population *pop, int couples);

int      drawLitterCount(void);
uint16_t drawLitterMask(int L);

#endif