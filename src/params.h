#ifndef PARAMS_H
#define PARAMS_H

/* Temps */
#define HORIZON_MONTHS   96     //simulation de 8 ans
#define MAX_AGE_MONTHS   180    /* mort certaine à 15 ans */

/* Population initiale */
#define INITIAL_COUPLES  1
#define INITIAL_AGE      12

/* Survie annuelle (convertie en mensuelle : s^(1/12)) */
#define SURVIVAL_YOUNG   0.35
#define SURVIVAL_ADULT   0.60
#define OLD_AGE_START    10     
#define OLD_AGE_STEP     0.10

/* Maturité (mois, bornes incluses) */
#define MATURITY_MIN     5
#define MATURITY_MAX     8

/* Reproduction */
#define LITTERS_MIN      3
#define LITTERS_MAX      9
#define KITS_MIN         3
#define KITS_MAX         6
#define P_FEMALE         0.5

/* Poids de la loi des portées/an pour L = 3..9 */
static const int LITTER_WEIGHTS[7] = {1, 1, 2, 2, 2, 1, 1};

#define BASE_SEED        12345UL

#endif