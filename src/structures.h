#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <stdint.h>

typedef enum {
    Male,
    Female
} Sex;

typedef struct {
    uint8_t  age;           // en mois
    uint8_t  maturity_age;  // tiré à la naissance, 5..8 
    uint8_t  sex;           
    uint8_t  litters_year;  //combien de portee en 12 mois
    uint16_t litter_mask;  //quelle mois sera la portee
} Rabbit;

typedef struct {
    Rabbit *rabbits;
    int count;
    int capacity;
} Population;

typedef struct {
    long long births;
    long long deaths;
    long long litters;      
} Monthly_Stats;

#endif