import sys
import csv
import matplotlib.pyplot as plt

# --- 0. Setup e Validazione Argomenti ---
if len(sys.argv) < 2:
    print("Error: Missing required argument. Correct syntax: python grafico_scaling.py ../PROGETTO_OUTPUT/metriche.csv")
    sys.exit(1)

# --- 1. Lettura dei Dati ---
data = []
with open(sys.argv[1], newline="") as f:
    for rows in csv.DictReader(f):
        record = {
            "nproc": int(rows["nproc"]),
            "nthread": int(rows["nthread"]),
            "core": int(rows["nproc"]) * int(rows["nthread"]),
            "NX": int(rows["NX"]), 
            "NY": int(rows["NY"]),
            "t_tot": float(rows["t_tot"]),
            "t_compute": float(rows["t_compute"]),
            "t_halo": float(rows["t_halo"]),
            "throughput": float(rows["throughput"])
        }
        data.append(record)

if not data:
    print("Error: No data found in the CSV file.")
    sys.exit(1)

# --- METRICHE --- #
# 1. Throughput: MUPS (Million Updates Per Second) al variare di processi × thread #
cores = [d["core"] for d in data]
mups = [d["throughput"] for d in data]

plt.figure(figsize=(7, 5))
plt.plot(cores, mups, "o-", label="Throughput (MUPS)")
plt.xlabel("Total Core (MPI Processes x OpenMP Threads)")
plt.ylabel("Million Updates Per Second (MUPS)")
plt.title("1. Throughput")
plt.grid(alpha=0.3)
plt.savefig("../PROGETTO_OUTPUT/1_throughput.png", dpi=120)
print("Saved ../PROGETTO_OUTPUT/1_throughput.png")

# --- METRICHE --- #
# 2. Strong scaling ed efficienza parallela #
strong_data = [d for d in data if d["NX"] == 1024 and d["NY"] == 1024]
if strong_data:
    strong_data.sort(key=lambda x: x["core"])
    c = [d["core"] for d in strong_data]
    t = [d["t_tot"] for d in strong_data]
    
    t1 = t[0] # Tempo di esecuzione sequenziale (o con 1 nodo/core)
    speedup = [t1 / ti for ti in t]
    ideale = [ci / c[0] for ci in c]
    efficienza = [100 * s / i for s, i in zip(speedup, ideale)]

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(11, 4.5))
    ax1.plot(c, speedup, "o-", label="Misurato")
    ax1.plot(c, ideale, "k--", label="Ideale")
    ax1.set_xlabel("Core totali"); ax1.set_ylabel("Speedup")
    ax1.set_title("2. Strong Scaling - Speedup"); ax1.legend(); ax1.grid(alpha=0.3)

    ax2.plot(c, efficienza, "s-", color="green")
    ax2.axhline(100, color="k", ls="--", alpha=0.6)
    ax2.set_xlabel("Core totali"); ax2.set_ylabel("Efficienza (%)")
    ax2.set_title("2. Strong Scaling - Efficienza"); ax2.set_ylim(0, 110); ax2.grid(alpha=0.3)
    fig.tight_layout(); fig.savefig("../PROGETTO_OUTPUT/2_strong_scaling.png", dpi=120)
    print("Salvato ../PROGETTO_OUTPUT/2_strong_scaling.png")

# --- METRICHE --- #
# 3. Weak scaling: aumentare la griglia proporzionalmente ai processi #
weak_data = [d for d in data if d["NX"] == 512 * d["nproc"] and d["NY"] == 512]
if weak_data:
    weak_data.sort(key=lambda x: x["core"])
    c = [d["core"] for d in weak_data]
    t = [d["t_tot"] for d in weak_data]
    
    plt.figure(figsize=(7, 5))
    plt.plot(c, t, "o-", color="purple")
    plt.xlabel("Core totali")
    plt.ylabel("Tempo totale (s)")
    plt.title("3. Weak Scaling (Tempo ideale costante)")
    plt.grid(alpha=0.3)
    plt.ylim(bottom=0)
    plt.savefig("../PROGETTO_OUTPUT/3_weak_scaling.png", dpi=120)
    print("Salvato ../PROGETTO_OUTPUT/3_weak_scaling.png")

# --- METRICHE --- #
# 4. Impatto della dimensione della griglia sul tempo di halo exchange vs tempo di calcolo #
if weak_data:
    etichette = [f"{d['NX']}x{d['NY']}" for d in weak_data]
    t_c = [d["t_compute"] for d in weak_data]
    t_h = [d["t_halo"] for d in weak_data]
    x = range(len(etichette))

    plt.figure(figsize=(7, 5))
    plt.bar(x, t_c, label="Tempo di calcolo")
    plt.bar(x, t_h, bottom=t_c, label="Tempo Halo Exchange")
    plt.xticks(list(x), etichette, rotation=15)
    plt.ylabel("Tempo (s)")
    plt.title("4. Impatto dimensione griglia su Halo vs Calcolo")
    plt.legend()
    plt.grid(axis='y', alpha=0.3)
    plt.savefig("../PROGETTO_OUTPUT/4_halo_vs_calcolo.png", dpi=120)
    print("Salvato ../PROGETTO_OUTPUT/4_halo_vs_calcolo.png")

print("Generazione completata in accordo alle Specifiche.pdf.")