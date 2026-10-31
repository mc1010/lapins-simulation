#include <stdio.h>
#include <math.h>
#include "structures.h"
#include "params.h"
#include "Random.h"
#include "mt19937ar.h"
#include "Population.h"
#include "Survival.h"
#include "simulation.h"

/* Compteurs pour le bilan final */
static int nbOk = 0;
static int nbTotal = 0;

/* ------------------------------------------------------------
   verifier : compare ce qu'on a obtenu à ce qu'on attendait.
   Si l'écart est <= tolerance, le test est OK, sinon ERREUR.
   (Les tirages aléatoires ne tombent jamais pile sur la valeur
   théorique, d'où la tolérance.)
   ------------------------------------------------------------ */
static void verifier(const char *nom, double obtenu, double attendu, double tolerance)
{
    int ok = fabs(obtenu - attendu) <= tolerance;

    nbTotal++;
    if (ok) nbOk++;
    printf("[%s] %-40s obtenu=%.4f attendu=%.4f\n",
           ok ? "OK" : "ERREUR", nom, obtenu, attendu);
}

/* Compte les bits à 1 d'un nombre (sert à tester le masque de portées) */
static int countBits(unsigned int m)
{
    int c = 0;
    while (m)
    {
        c += m & 1;
        m >>= 1;
    }
    return c;
}

/* ------------------------------------------------------------
   TEST 1 : le Mersenne Twister est bien copié.
   Avec la graine 5489, le tout premier nombre doit être 3499211612.
   ------------------------------------------------------------ */
static void test_mersenne_twister(void)
{
    init_genrand(5489UL);
    verifier("MT: premier tirage (graine 5489)", (double)genrand_int32(), 3499211612.0, 0.0);
}

/* TEST 2 : la structure Rabbit fait bien 6 octets. */
static void test_taille_rabbit(void)
{
    verifier("sizeof(Rabbit)", (double)sizeof(Rabbit), 6.0, 0.0);
}

/* ------------------------------------------------------------
   TEST 3 : randInt(5, 8) donne bien 5, 6, 7 ou 8, à égalité.
   On tire 1 million de fois : le plus petit doit être 5, le plus
   grand 8, et la moyenne 6.5 (= (5+6+7+8)/4).
   ------------------------------------------------------------ */
static void test_randInt(void)
{
    long i;
    int v, minVu = 100, maxVu = -100;
    double somme = 0.0;

    initRandom(BASE_SEED);
    for (i = 0; i < 1000000; i++)
    {
        v = randInt(5, 8);
        somme += v;
        if (v < minVu) minVu = v;
        if (v > maxVu) maxVu = v;
    }
    verifier("randInt(5,8): plus petit", minVu, 5, 0);
    verifier("randInt(5,8): plus grand", maxVu, 8, 0);
    verifier("randInt(5,8): moyenne", somme / 1000000, 6.5, 0.02);
}

/* TEST 4 : randomSex donne ~50 % de femelles (P_FEMALE = 0.5). */
static void test_sexe(void)
{
    long i, femelles = 0;

    initRandom(BASE_SEED);
    for (i = 0; i < 1000000; i++)
        if (randomSex() == Female) femelles++;
    verifier("randomSex: proportion de femelles", femelles / 1000000.0, P_FEMALE, 0.005);
}

/* ------------------------------------------------------------
   TEST 5 : le nombre de portées par an (L).
   La moyenne doit être 6, et L doit valoir 5, 6 ou 7 dans 60 %
   des cas (poids 2+2+2 sur un total de 10).
   ------------------------------------------------------------ */
static void test_nombre_portees(void)
{
    long i, dans567 = 0;
    int L;
    double somme = 0.0;

    initRandom(BASE_SEED);
    for (i = 0; i < 1000000; i++)
    {
        L = drawLitterCount();
        somme += L;
        if (L >= 5 && L <= 7) dans567++;
    }
    verifier("portees/an: moyenne de L", somme / 1000000, 6.0, 0.02);
    verifier("portees/an: part de L dans {5,6,7}", dans567 / 1000000.0, 0.6, 0.01);
}

/* ------------------------------------------------------------
   TEST 6 : le masque de portées.
   Pour chaque L de 3 à 9, on tire 100000 masques. Chacun doit
   avoir exactement L bits à 1, et rester sur 12 bits (< 4096).
   Si UN SEUL masque est faux, le test est en erreur.
   ------------------------------------------------------------ */
static void test_masque(void)
{
    int L, i, ok = 1;
    uint16_t m;

    initRandom(BASE_SEED);
    for (L = LITTERS_MIN; L <= LITTERS_MAX; L++)
        for (i = 0; i < 100000; i++)
        {
            m = drawLitterMask(L);
            if (countBits(m) != L) ok = 0;
            if (m >= 4096) ok = 0;
        }
    verifier("masque: toujours L bits, sur 12 bits", ok, 1, 0);
}

/* ------------------------------------------------------------
   TEST 7 : les couples de départ.
   3 couples = 6 lapins, tous âgés de 12 mois, dont exactement
   3 femelles. Le premier de chaque couple est un mâle.
   ------------------------------------------------------------ */
static void test_couples_initiaux(void)
{
    Population pop;
    int i, femelles = 0;

    initRandom(BASE_SEED);
    initPopulation(&pop, 4);
    addInitialCouples(&pop, 3);

    for (i = 0; i < pop.count; i++)
        if (pop.rabbits[i].sex == Female) femelles++;

    verifier("couples: nombre de lapins", pop.count, 6, 0);
    verifier("couples: age = maturite + 11", pop.rabbits[0].age - pop.rabbits[0].maturity_age, 11, 0);
    verifier("couples: nombre de femelles", femelles, 3, 0);
    verifier("couples: premier lapin = male (0)", pop.rabbits[0].sex, Male, 0);

    freePopulation(&pop);
}

/* ------------------------------------------------------------
   TEST 8 : les taux de survie MENSUELS de la table.
   Ce sont les taux annuels convertis : s_mois = s_an ^ (1/12).
   ------------------------------------------------------------ */
static void test_survie_mensuelle(void)
{
    initSurvivalTable();
    verifier("survie mensuelle: jeune (35% par an)", getMonthlySurvival(3, 0), 0.9162, 0.0005);
    verifier("survie mensuelle: adulte (60% par an)", getMonthlySurvival(30, 1), 0.9583, 0.0005);
    verifier("survie mensuelle: adulte a 10 ans (50%)", getMonthlySurvival(120, 1), 0.9439, 0.0005);
    verifier("survie mensuelle: adulte a 14 ans (10%)", getMonthlySurvival(168, 1), 0.8254, 0.0005);
    verifier("survie mensuelle: a 180 mois (mort)", getMonthlySurvival(180, 1), 0.0, 0.0);
}

/* ------------------------------------------------------------
   Aide pour le test 9 : on suit 100000 lapins pendant 12 mois.
   Chaque mois, chaque lapin "tire sa chance" avec sa probabilité
   de survie. Au premier échec il est mort. On renvoie la part de
   ceux qui ont passé les 12 mois.
   ageDepart = âge du premier mois suivi, adulte = 0 ou 1.
   ------------------------------------------------------------ */
static double fractionSurvivants(int ageDepart, int adulte)
{
    long k, vivants = 0;
    int age, ok;

    for (k = 0; k < 100000; k++)
    {
        ok = 1;
        for (age = ageDepart; age < ageDepart + 12 && ok; age++)
            if (!randChance(getMonthlySurvival(age, adulte)))
                ok = 0;
        if (ok) vivants++;
    }
    return vivants / 100000.0;
}

/* TEST 9 : sur 12 mois, 60 % des adultes et 35 % des jeunes survivent. */
static void test_cohortes(void)
{
    initRandom(BASE_SEED);
    verifier("cohorte adulte sur 12 mois", fractionSurvivants(13, 1), 0.60, 0.01);
    verifier("cohorte jeune sur 12 mois", fractionSurvivants(1, 0), 0.35, 0.01);
}

/* ------------------------------------------------------------
   TEST 10 : la mortalité.
   a) 100000 adultes pendant 12 mois : ~60 % survivent, et
      survivants + morts = 100000 (personne ne disparaît sans
      être compté).
   b) Des lapins de 179 mois : après 1 mois ils ont 180 mois,
      donc ils meurent tous.
   c) Le compteur de mâles matures est juste : on le compare à
      un recomptage fait à la main.
   ------------------------------------------------------------ */
static void test_mortalite(void)
{
    Population pop;
    Monthly_Stats stats;
    int i, mois, matureMales, recompte = 0;

    /* a) */
    initRandom(BASE_SEED);
    initPopulation(&pop, 1000);
    stats.births = 0; stats.deaths = 0; stats.litters = 0;
    for (i = 0; i < 100000; i++)
        addRabbit(&pop, createRabbitOfSex(24, Female));
    for (mois = 0; mois < 12; mois++)
    {
        agePopulation(&pop);
        handleMortality(&pop, &stats, &matureMales);
    }
    verifier("mortalite: part de survivants a 12 mois", pop.count / 100000.0, 0.60, 0.01);
    verifier("mortalite: survivants + morts = depart", pop.count + stats.deaths, 100000, 0);
    freePopulation(&pop);

    /* b) */
    initPopulation(&pop, 1000);
    stats.deaths = 0;
    for (i = 0; i < 1000; i++)
        addRabbit(&pop, createRabbitOfSex(179, Female));
    agePopulation(&pop);
    handleMortality(&pop, &stats, &matureMales);
    verifier("mortalite: lapins de 180 mois tous morts", pop.count, 0, 0);
    freePopulation(&pop);

    /* c) */
    initPopulation(&pop, 1000);
    stats.deaths = 0;
    for (i = 0; i < 5000; i++)
        addRabbit(&pop, createRabbitOfSex(24, (i % 2) ? Male : Female));
    agePopulation(&pop);
    handleMortality(&pop, &stats, &matureMales);
    for (i = 0; i < pop.count; i++)
        if (pop.rabbits[i].sex == Male && pop.rabbits[i].age >= pop.rabbits[i].maturity_age)
            recompte++;
    verifier("mortalite: compteur de males matures", matureMales, recompte, 0);
    freePopulation(&pop);
}

/* ------------------------------------------------------------
   TEST 11 : la reproduction.
   a) 10000 femelles qui entrent en cycle (age = maturite - 1, donc
      après 1 mois m = 0), suivies 12 mois, avec un mâle présent.
      On supprime les nouveau-nés à chaque mois pour ne compter que
      les 10000 mères. Attendu : 6 portées par mère et par an,
      4.5 petits par portée, 50 % de femelles, âge 0 à la naissance.
   b) Même chose SANS mâle mature : aucune naissance.
   ------------------------------------------------------------ */
static void test_reproduction(void)
{
    Population pop;
    Monthly_Stats stats;
    int i, mois, N = 10000, agesOk = 1;
    long long nes = 0, femelles = 0;

    /* a) avec un mâle */
    initRandom(BASE_SEED);
    initPopulation(&pop, 100);
    for (i = 0; i < N; i++)
    {
        Rabbit r = createRabbitOfSex(0, Female);
        r.age = (uint8_t)(r.maturity_age - 1);
        addRabbit(&pop, r);
    }
    stats.births = 0; stats.deaths = 0; stats.litters = 0;

    for (mois = 0; mois < 12; mois++)
    {
        agePopulation(&pop);
        handleReproduction(&pop, &stats, 1);
        for (i = N; i < pop.count; i++)
        {
            nes++;
            if (pop.rabbits[i].sex == Female) femelles++;
            if (pop.rabbits[i].age != 0) agesOk = 0;
        }
        pop.count = N;   /* on enlève les nouveau-nés */
    }
    verifier("repro: portees par mere et par an", (double)stats.litters / N, 6.0, 0.05);
    verifier("repro: petits par portee", (double)stats.births / stats.litters, 4.5, 0.03);
    verifier("repro: nouveau-nes comptes = naissances", (double)nes, (double)stats.births, 0);
    verifier("repro: part de femelles parmi les petits", (double)femelles / nes, 0.5, 0.01);
    verifier("repro: nouveau-nes d'age 0", agesOk, 1, 0);
    freePopulation(&pop);

    /* b) sans mâle mature */
    initPopulation(&pop, 100);
    for (i = 0; i < 1000; i++)
    {
        Rabbit r = createRabbitOfSex(0, Female);
        r.age = (uint8_t)(r.maturity_age - 1);
        addRabbit(&pop, r);
    }
    stats.births = 0; stats.deaths = 0; stats.litters = 0;
    for (mois = 0; mois < 12; mois++)
    {
        agePopulation(&pop);
        handleReproduction(&pop, &stats, 0);
    }
    verifier("repro: naissances sans male mature", (double)stats.births, 0, 0);
    freePopulation(&pop);
}
int main(void)
{
    test_mersenne_twister();
    test_taille_rabbit();
    test_randInt();
    test_sexe();
    test_nombre_portees();
    test_masque();
    test_couples_initiaux();
    test_survie_mensuelle();
    test_cohortes();
    test_mortalite();
    test_reproduction();

    printf("\nBilan : %d tests OK sur %d\n", nbOk, nbTotal);
    return 0;
}