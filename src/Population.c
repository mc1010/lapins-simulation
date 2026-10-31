#include <stdio.h>
#include <stdlib.h>
#include "structures.h"
#include "Population.h"
#include "Random.h"
#include "params.h"

int initPopulation(Population *pop, int initialCapacity)
{
    if (initialCapacity <= 0) { initialCapacity = 4; }
    pop->rabbits = malloc(initialCapacity * sizeof(Rabbit));
    if (pop->rabbits == NULL)
    {
        pop->count = 0;
        pop->capacity = 0;
        return 0;
    }
    pop->count = 0;
    pop->capacity = initialCapacity;
    return 1;
}

int addRabbit(Population *pop, Rabbit r)
{
    if (pop->count == pop->capacity)
    {
        int newCapacity;
        if (pop->capacity == 0)
            newCapacity = 4;
        else
            newCapacity = pop->capacity * 2;
        Rabbit *temp = realloc(pop->rabbits, newCapacity * sizeof(Rabbit));
        if (temp == NULL) return 0;
        pop->rabbits = temp;
        pop->capacity = newCapacity;
    }

    pop->rabbits[pop->count] = r;
    pop->count++;
    return 1;
}

void removeRabbit(Population *pop, int index)
{
    if (index < 0 || index >= pop->count) return;
    pop->rabbits[index] = pop->rabbits[pop->count - 1];
    pop->count--;
}

void freePopulation(Population *pop)
{
    free(pop->rabbits);
    pop->rabbits = NULL;
    pop->count = 0;
    pop->capacity = 0;
}

/* ---------- Création des lapins ---------- */

// Lapin d'un sexe imposé : maturité tirée, cycle de portées pas encore tiré.
Rabbit createRabbitOfSex(int age, Sex sex)
{
    Rabbit r;
    r.age          = (uint8_t)age;
    r.sex          = (uint8_t)sex;
    r.maturity_age = (uint8_t)randInt(MATURITY_MIN, MATURITY_MAX);
    r.litters_year = 0;
    r.litter_mask  = 0;   /* 0 = cycle pas encore tiré */
    return r;
}

//Lapin dont le sexe est tiré (naissances).
Rabbit createRabbit(int age)
{
    return createRabbitOfSex(age, randomSex());
}

// Ajoute des couples adultes avec l'age initial
int addInitialCouples(Population *pop, int couples)
{
    int i;
    for (i = 0; i < couples; i++)
    {
        Rabbit male   = createRabbitOfSex(0, Male);
        Rabbit female = createRabbitOfSex(0, Female);

        male.age   = (uint8_t)(male.maturity_age + 11);
        female.age = (uint8_t)(female.maturity_age + 11);

        if (!addRabbit(pop, male))   return 0;
        if (!addRabbit(pop, female)) return 0;
    }
    return 1;
}

//---------- Cycle de portées ---------- 

// Tire L dans 3..9 selon LITTER_WEIGHTS. 
int drawLitterCount(void)
{
    int total = 0, i, r;
    for (i = 0; i < 7; i++) total += LITTER_WEIGHTS[i];

    r = randInt(1, total);
    for (i = 0; i < 7; i++)
    {
        r -= LITTER_WEIGHTS[i];
        if (r <= 0) return LITTERS_MIN + i;
    }
    return LITTERS_MAX;   
}

// Tire L mois distincts parmi 12 : un bit à 1 par mois de portée.
uint16_t drawLitterMask(int L)
{
    uint16_t mask = 0;
    int count = 0;
    while (count < L)
    {
        int month = randInt(0, 11);
        if (!((mask >> month) & 1))
        {
            mask = (uint16_t)(mask | (1u << month));
            count++;
        }
    }
    return mask;
}