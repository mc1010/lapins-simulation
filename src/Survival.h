#ifndef SURVIVAL_H
#define SURVIVAL_H

// Calcule la table de survie mensuelle
void initSurvivalTable(void);

// Probabilité de survivre ce mois-ci, pour un lapin d'âge 'age' (mois).
double getMonthlySurvival(int age, int adult);

#endif