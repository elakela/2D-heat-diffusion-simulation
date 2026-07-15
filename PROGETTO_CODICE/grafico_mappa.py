# DELIVERABLE:
# Script Python per la visualizzazione della mappa di calore 

import sys
import numpy as np
import matplotlib.pyplot as plt

if len(sys.argv) < 2:
    print("Error: Missing required argument. Correct syntax: python grafico_mappa.py ../PROGETTO_OUTPUT/calore_finale.csv")
    sys.exit(1)

filename = sys.argv[1]

grid = np.loadtxt(filename, delimiter=",")

plt.figure(figsize=(6, 5))
plt.imshow(grid, cmap="hot", origin="upper", vmin=0, vmax=100)
plt.colorbar(label="Temperature")
plt.title("Heat diffusion - " + filename)
plt.xlabel("column")
plt.ylabel("row")
plt.tight_layout()

output_png = "../PROGETTO_OUTPUT/" + filename.replace(".csv", ".png")
plt.savefig(output_png, dpi=120)
print("Image saved as: ../PROGETTO_OUTPUT" + filename)