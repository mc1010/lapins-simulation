import matplotlib
matplotlib.use("Agg")   

import pandas as pd
import matplotlib.pyplot as plt
import os

os.makedirs("figures", exist_ok=True)


ages = pd.read_csv("ages_final.csv")
ages = ages[(ages["males"] + ages["females"]) > 0]

fig, ax = plt.subplots(figsize=(8, 4))
ax.bar(ages["age"], ages["females"], label="Femelles", color="#c0504d")
ax.bar(ages["age"], ages["males"], bottom=ages["females"], label="Mâles", color="#4f81bd")
ax.set_xlabel("Âge (mois)")
ax.set_ylabel("Nombre de lapins")
ax.set_title("Répartition des âges à la fin de la simulation")
ax.legend()
fig.tight_layout()
fig.savefig("figures/fig_ages.png", dpi=200)

# --- Age au deces ---
dec = pd.read_csv("ages_deces.csv")
dec = dec[(dec["deces_jeunes"] + dec["deces_adultes"]) > 0]

fig, ax = plt.subplots(figsize=(8, 4))
ax.bar(dec["age"], dec["deces_jeunes"], label="Jeunes (avant maturité)", color="#9bbb59")
ax.bar(dec["age"], dec["deces_adultes"], bottom=dec["deces_jeunes"], label="Adultes", color="#8064a2")
ax.set_xlabel("Âge au décès (mois)")
ax.set_ylabel("Nombre de décès")
ax.set_title("Âge des lapins au moment de leur décès")
ax.legend()
fig.tight_layout()
fig.savefig("figures/fig_deces.png", dpi=200)

# --- Quelques chiffres pour le rapport ---
total = dec["deces_jeunes"].sum() + dec["deces_adultes"].sum()
print("Deces totaux :", total)
print("Part de jeunes parmi les morts : %.1f %%" % (100 * dec["deces_jeunes"].sum() / total))
print("Age moyen au deces : %.1f mois" % ((dec["age"] * (dec["deces_jeunes"] + dec["deces_adultes"])).sum() / total))
pop = ages["males"].sum() + ages["females"].sum()
print("Population finale :", pop)
print("Part de femelles : %.1f %%" % (100 * ages["females"].sum() / pop))
print("Age moyen des vivants : %.1f mois" % ((ages["age"] * (ages["males"] + ages["females"])).sum() / pop))
print("Figures ecrites : figures/fig_ages.png, figures/fig_deces.png")