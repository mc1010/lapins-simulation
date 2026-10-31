#ifndef MT19937AR_H
#define MT19937AR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Initialise l'état du générateur avec une graine. */
void init_genrand(unsigned long s);

/* Initialise l'état avec un tableau de graines. */
void init_by_array(unsigned long init_key[], int key_length);

/* Entier aléatoire dans [0, 0xffffffff]. */
unsigned long genrand_int32(void);

/* Entier aléatoire dans [0, 0x7fffffff]. */
long genrand_int31(void);

/* Réel aléatoire dans [0, 1]. */
double genrand_real1(void);

/* Réel aléatoire dans [0, 1[  (celui à utiliser pour randInt / randChance). */
double genrand_real2(void);

/* Réel aléatoire dans ]0, 1[. */
double genrand_real3(void);

/* Réel aléatoire dans [0, 1[ avec une résolution de 53 bits. */
double genrand_res53(void);

#ifdef __cplusplus
}
#endif

#endif