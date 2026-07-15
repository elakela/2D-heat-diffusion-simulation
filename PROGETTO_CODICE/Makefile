CC       = mpicc
CFLAGS   = -O3 -fopenmp -Wall
LIBS  = -lm

# Parametri di default configurabili da riga di comando
NX          ?= 1024
NY          ?= 1024
ITER        ?= 5000
ALPHA       ?= 0.25
NP          ?= 4
OMP_THREADS ?= 2

.PHONY: prova pulisci

# "make" senza argomenti compila il programma
diffusione: diffusione_calore.c
	$(CC) $(CFLAGS) -o diffusione diffusione_calore.c $(LIBS)

# "make proof" compila ed esegue una simulazione di esempio
proof: diffusione
	OMP_NUM_THREADS=$(OMP_THREADS) mpirun -np $(NP) ./diffusione $(NX) $(NY) $(ITER) --diffusion $(ALPHA) --temp-edges $(TEMP_EDGES) --temp-source $(TEMP_SOURCE) --save-each $(SAVE_EACH)

# "make clean" rimuove eseguibile e file prodotti
clean:
	rm -rf ./diffusione ../"PROGETTO_OUTPUT"
