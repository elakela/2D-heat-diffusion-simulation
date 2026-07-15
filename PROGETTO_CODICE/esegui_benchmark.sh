#!/bin/bash

echo "Compilazione del progetto..."
make

# Rimuove solo le metriche precedenti per iniziare da zero
rm -f ../PROGETTO_OUTPUT/metriche.csv

ITER=1000
ALPHA=0.25

echo ""
echo "======================================"
echo "--- Inizio test di STRONG SCALING ---"
echo "======================================"
# Griglia fissa, si aumentano i processi MPI mantenendo costanti/variando i thread OpenMP
NX=1024
NY=1024

for NP in 1 2 4; do
    for THREADS in 1 2; do
        echo "Eseguo con $NP processi MPI e $THREADS thread OpenMP (Griglia $NX x $NY)..."
        OMP_NUM_THREADS=$THREADS mpirun -np $NP ./diffusione $NX $NY $ITER -a $ALPHA
    done
done

echo ""
echo "======================================"
echo "--- Inizio test di WEAK SCALING ---"
echo "======================================"
# Si aumenta la dimensione della griglia all'aumentare dei processi
# Carico per processo costante = 512 righe per processo
NY_WEAK=512

for NP in 1 2 4; do
    NX_WEAK=$((512 * NP))
    echo "Eseguo con $NP processi MPI e 1 thread OpenMP (Griglia $NX_WEAK x $NY_WEAK)..."
    OMP_NUM_THREADS=1 mpirun -np $NP ./diffusione $NX_WEAK $NY_WEAK $ITER -a $ALPHA
done

echo ""
echo "--- Benchmark completati! I risultati sono stati salvati in ../PROGETTO_OUTPUT/metriche.csv ---"
