# OUTPUT: 
# Errore di convergenza in funzione delle iterazioni

import sys
import numpy as np
import matplotlib.pyplot as plt

if len(sys.argv) < 2:
    print("Error: Missing required argument. Correct syntax: python grafico_convergenza.py ../PROGETTO_OUTPUT/convergenza.csv [altri.csv ...]")
    sys.exit(1)

plt.figure(figsize=(7, 5))

for filename in sys.argv[1:]:
    # salto la prima riga (intestazione "iterazione,errore")
    data = np.loadtxt(filename, delimiter=",", skiprows=1)
    iter = data[:, 0]
    error = data[:, 1]
    plt.semilogy(iter, error, label=filename)

plt.xlabel("iterations")
plt.ylabel("max error |new_temp - old_temp|")
plt.title("Convergence to steady state")
plt.grid(True, which="both", alpha=0.3)
plt.legend()
plt.tight_layout()
plt.savefig("../PROGETTO_OUTPUT/convergenza.png", dpi=120)
print("Graph saved as: ../PROGETTO_OUTPUT/convergenza.png")