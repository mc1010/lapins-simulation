import glob
import os

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

os.makedirs("figures", exist_ok=True)


def save(fig, name):
    path = os.path.join("figures", name)
    fig.tight_layout()
    fig.savefig(path, dpi=150)
    plt.close(fig)
    print("ecrit :", path)


# ----------------------------------------------------------------------
# 1) Experiences multi-replications (lapins exp)
# ----------------------------------------------------------------------
if os.path.exists("exp_courbes.csv"):
    cur = pd.read_csv("exp_courbes.csv")
    res = pd.read_csv("exp_resume.csv")
    runs = pd.read_csv("exp_runs.csv")
    conds = sorted(cur["couples"].unique())
    cmap = plt.get_cmap("viridis")
    colors = {c: cmap(i / max(1, len(conds) - 1) * 0.9) for i, c in enumerate(conds)}
    reps = int(res["reps"].iloc[0])

    # Courbes de croissance : mediane + bande Q1-Q3 (runs encore vivants), echelle log
    fig, ax = plt.subplots(figsize=(8, 5))
    for c in conds:
        d = cur[(cur["couples"] == c) & (cur["n_vivants"] > 0)]
        ax.plot(d["mois"], d["mediane"], color=colors[c], label=f"{c} couple(s) ({2 * c} lapins)")
        ax.fill_between(d["mois"], d["q1"], d["q3"], color=colors[c], alpha=0.2)
    ax.set_yscale("log")
    ax.set_xlabel("Temps (mois)")
    ax.set_ylabel("Nombre de lapins (echelle log)")
    ax.set_title(f"Croissance de la population - mediane et quartiles ({reps} replications)")
    ax.grid(True, which="both", alpha=0.3)
    ax.legend()
    save(fig, "fig1_croissance.png")

    # Proportion d'extinctions selon le nombre de couples (barres d'erreur de Wilson)
    fig, ax = plt.subplots(figsize=(7, 4.5))
    x = np.arange(len(res))
    y = res["extinction_pct"].to_numpy()
    err = np.vstack([y - res["wilson_bas_pct"], res["wilson_haut_pct"] - y])
    ax.bar(x, y, yerr=err, capsize=5, color="#4c72b0", alpha=0.85)
    ax.set_xticks(x)
    ax.set_xticklabels([str(c) for c in res["couples"]])
    ax.set_xlabel("Couples au depart")
    ax.set_ylabel("Runs eteints (%)")
    ax.set_title("Probabilite d'extinction (IC 95 % de Wilson)")
    ax.grid(True, axis="y", alpha=0.3)
    save(fig, "fig2_extinction.png")

    # Extinctions au cours du temps
    fig, ax = plt.subplots(figsize=(8, 4.5))
    for c in conds:
        d = cur[cur["couples"] == c]
        ax.step(d["mois"], 100 * d["frac_extinction"], where="post", color=colors[c], label=f"{c} couple(s)")
    ax.set_xlabel("Temps (mois)")
    ax.set_ylabel("Runs eteints (%)")
    ax.set_title("Quand les populations s'eteignent-elles ?")
    ax.grid(True, alpha=0.3)
    ax.legend()
    save(fig, "fig3_extinction_temps.png")

    # Distribution des effectifs finaux (survivants a l'horizon)
    surv = runs[runs["arret"] == "horizon"]
    fig, ax = plt.subplots(figsize=(7, 4.5))
    data = [surv[surv["couples"] == c]["effectif_final"].to_numpy() for c in conds]
    ax.boxplot(data, tick_labels=[str(c) for c in conds], showfliers=True)
    ax.set_yscale("log")
    ax.set_xlabel("Couples au depart")
    ax.set_ylabel("Effectif final (echelle log)")
    ax.set_title("Variabilite de l'effectif final entre replications")
    ax.grid(True, which="both", axis="y", alpha=0.3)
    save(fig, "fig4_effectifs_finaux.png")

    # Temps de calcul total par condition
    fig, ax = plt.subplots(figsize=(7, 4.5))
    ax.bar(x, res["secondes_total"], color="#dd8452", alpha=0.9)
    ax.set_xticks(x)
    ax.set_xticklabels([str(c) for c in res["couples"]])
    ax.set_xlabel("Couples au depart")
    ax.set_ylabel(f"Temps pour {reps} runs (s)")
    ax.set_title("Cout de calcul selon la condition initiale")
    ax.grid(True, axis="y", alpha=0.3)
    save(fig, "fig5_temps_calcul.png")

    print("\nResume des conditions :")
    print(res[["couples", "reps", "extinctions", "extinction_pct", "mediane", "q1", "q3"]].to_string(index=False))

# ----------------------------------------------------------------------
# 2) Limite du programme (lapins limit) : population, temps, memoire
# ----------------------------------------------------------------------
files = sorted(glob.glob("limite_c*_s*.csv"), key=os.path.getmtime)
if files:
    d = pd.read_csv(files[-1])
    fig, axes = plt.subplots(1, 3, figsize=(14, 4.2))

    axes[0].plot(d["month"], d["total"], color="#4c72b0")
    axes[0].set_yscale("log")
    axes[0].set_title("Population (echelle log)")
    axes[0].set_xlabel("Mois")

    axes[1].plot(d["month"], d["seconds"], color="#dd8452")
    axes[1].set_yscale("log")
    axes[1].set_title("Temps de calcul cumule (s, log)")
    axes[1].set_xlabel("Mois")

    axes[2].plot(d["month"], d["memory_mb"], color="#55a868")
    axes[2].set_yscale("log")
    axes[2].set_title("Memoire du tableau (Mo, log)")
    axes[2].set_xlabel("Mois")

    for a in axes:
        a.grid(True, which="both", alpha=0.3)
    fig.suptitle(f"Limite du programme ({os.path.basename(files[-1])})")
    save(fig, "fig6_limite.png")

    # Taux de croissance mensuel et annuel
    dd = d[d["total"] > 1000].copy()
    dd["taux_mensuel"] = dd["total"].pct_change()
    taux = dd["taux_mensuel"].iloc[-24:].mean()
    print(f"\nCroissance moyenne (24 derniers mois) : +{100 * taux:.1f} %/mois, soit x{(1 + taux) ** 12:.1f} par an")

    # Naissances et deces
    fig, ax = plt.subplots(figsize=(8, 4.5))
    ax.plot(d["month"], d["births"].clip(lower=1), label="naissances")
    ax.plot(d["month"], d["deaths"].clip(lower=1), label="deces")
    ax.set_yscale("log")
    ax.set_xlabel("Mois")
    ax.set_ylabel("Evenements par mois (log)")
    ax.set_title("Naissances et deces mensuels")
    ax.grid(True, which="both", alpha=0.3)
    ax.legend()
    save(fig, "fig7_naissances_deces.png")

    # Structure de la population (sexe)
    fig, ax = plt.subplots(figsize=(8, 4.5))
    part = 100 * d["females"] / d["total"].replace(0, np.nan)
    ax.plot(d["month"], part, color="#c44e52")
    ax.axhline(50, color="gray", ls="--")
    ax.set_ylim(0, 100)
    ax.set_xlabel("Mois")
    ax.set_ylabel("Femelles (%)")
    ax.set_title("Proportion de femelles au cours du temps")
    ax.grid(True, alpha=0.3)
    save(fig, "fig8_sexe.png")

if not os.path.exists("exp_courbes.csv") and not files:
    print("Aucun CSV trouve. Lance d'abord :  lapins exp  et/ou  lapins limit")