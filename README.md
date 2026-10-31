# Simulation stochastique d'une population de lapins

Simulation individu-centrée en C : chaque lapin a son âge, son sexe, sa maturité et son calendrier de portées ; naissances et décès sont tirés au hasard, au pas d'un mois.

## Contenu

| Dossier | Rôle |
|---|---|
| `src/` | programme `lapin.c` et modules (population, survie, simulation, générateur Mersenne Twister) |
| `tests/` | tests de validation |
| `scripts/` | `analyse.py` et `figures_ages.py` (figures à partir des CSV) |

## Compilation (PowerShell, GCC)

```powershell
mkdir bin
.\build.ps1
```

## Utilisation

```powershell
bin\lapins run   [mois] [graine] [couples] [plafond_millions]   # un run détaillé
bin\lapins limit [couples] [graine] [plafond_millions]          # limite du modèle libre
bin\lapins exp   [reps] [mois] [plafond_millions] [couples ...] # réplications
```
Exemples :

```powershell
bin\lapins run 240 4 1 50 K=100000
bin\lapins exp 1000 72 50 1 2 3 4 5
```

Les fichiers CSV sont écrits dans le dossier courant. Pour les figures : `python scripts\analyse.py`, à lancer dans le dossier qui contient les CSV.
