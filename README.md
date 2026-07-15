# 🌡️ 2D Heat Diffusion Simulation (MPI + OpenMP)

![C](https://img.shields.io/badge/Language-C-blue.svg)
![MPI](https://img.shields.io/badge/Parallel-MPI%20%7C%20OpenMP-orange.svg)
![Python](https://img.shields.io/badge/Scripts-Python-yellow.svg)

Questo repository contiene il progetto per il corso di **Principi della Programmazione Parallela** (A.A. 2025/2026).

Il progetto implementa una simulazione della diffusione del calore su una griglia bidimensionale, risolta tramite il metodo delle **differenze finite esplicite**. L'algoritmo è stato ottimizzato per architetture HPC (High Performance Computing) utilizzando un approccio di parallelizzazione **ibrido**:
- **MPI**: per la decomposizione del dominio a memoria distribuita e lo scambio dei bordi (halo exchange).
- **OpenMP**: per il calcolo locale intensivo (stencil multithread) sfruttando la memoria condivisa del singolo nodo.

---

## 📂 Struttura del Progetto

Il repository è diviso in due sezioni logiche:

### 1. `PROGETTO CODICE/`
Contiene tutto il codice sorgente, gli script di automazione e i visualizzatori.
- **`diffusione_calore.c`**: Il core della simulazione (motore C ibrido).
- **`Makefile`**: Automazione per la compilazione (`make`) e il testing rapido (`make prova`).
- **`esegui_benchmark.sh`**: Script Bash per l'esecuzione automatizzata delle prove di strong e weak scaling.
- **`grafico_*.py`**: Suite di script Python per generare le mappe di calore, le curve di convergenza e i grafici di efficienza partendo dai file `.csv`.

### 2. `PROGETTO RELAZIONE/`
Contiene la documentazione e l'analisi tecnica del lavoro svolto.
- **`Relazione.md`**: Trattazione completa
- **`Specifiche.pdf`**: Presentazione del progetto
Entrambi i file contengono: descrizione dello stencil, architettura parallela, grafici di scaling, analisi della convergenza

---

## 🚀 Guida all'Uso

### Prerequisiti
Per eseguire il codice e gli script sono necessari:
- Un ambiente MPI con compilatore C (es. `OpenMPI` con `mpicc`).
- `Python 3` con i pacchetti `numpy` e `matplotlib`.
*(Su ambiente Windows si raccomanda l'uso di WSL - Windows Subsystem for Linux).*

### Compilazione
Spostarsi nella cartella del codice e lanciare il comando `make`:

```bash
cd "PROGETTO CODICE"
make
```

### Esecuzione Base
Il programma richiede dimensioni della griglia (`NX`, `NY`) e il numero di iterazioni. È possibile configurare tramite flag il coefficiente di diffusione e le condizioni al contorno:

```bash
OMP_NUM_THREADS=4 mpirun -np 4 ./diffusione_calore 1024 1024 5000
```
*(Eseguire `./diffusione` senza argomenti per stampare l'elenco completo delle opzioni).*

### Generazione Benchmark e Grafici
Per avviare l'intera suite di test per lo scaling e ottenere in automatico tutti i grafici PNG richiesti:

```bash
python3 grafico_mappa.py ../PROGETTO_OUTPUT/calore_finale.csv
python3 grafico_convergenza.py ../PROGETTO_OUTPUT/convergenza.csv
chmod +x esegui_benchmark.sh
./esegui_benchmark.sh
python3 grafico_scaling.py ../PROGETTO_OUTPUT/metriche.csv
```

---

## 📈 Metriche e Analisi

La simulazione è in grado di calcolare dinamicamente:
- **Throughput in MUPS** (Million Updates Per Second).
- **Efficienza Parallela** e **Speedup** (Legge di Amdahl e Gustafson).
- Overhead della comunicazione: **Tempo di Calcolo vs Tempo di Halo Exchange**.

Tutti i risultati numerici vengono salvati in un log centralizzato (`metriche.csv`) pronto per essere analizzato dai tool Python inclusi nel repository.

---
*Progetto realizzato per l'insegnamento di Principi della Programmazione Parallela anno accademico 2025/2026 dalla studentessa Gabriela Riscica.*
