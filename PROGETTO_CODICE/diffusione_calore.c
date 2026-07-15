// DELIVERABLE
// ● Codice C/C++ commentato con Makefile
#define _GNU_SOURCE
#include <getopt.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <mpi.h>
#include <sys/stat.h>
#include <sys/types.h>

#define TOLERANCE 1e-5

void save_grid(const char *filename, double *grid, int NX, int NY) {
    FILE *f = fopen(filename, "w");
    if (f == NULL) return;
    for (int i = 0; i < NX; i++) {
        for (int j = 0; j < NY; j++) {
            if(j> 0) fprintf(f, ",");
            fprintf(f, "%.4f", grid[i * NY + j]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

/*
    // METRICHE DA RIPORTARE //
    1. Throughput: MUPS (Million Updates Per Second) 
    al variare di processi x thread
*/
void save_metrics(int NX, int NY, int nproc, int n_thread, double alpha, 
                  int iter_done, double global_error, 
                  double global_time, double compute_time, double halo_time) {
    /* MUPS = milioni di aggiornamenti di celle al secondo */
    double upload = (double) NX * NY * iter_done;
    double mups = (upload / global_time) / 1e6;
    
    printf("\n--- Diffusione del calore 2D (MPI + OpenMP) ---\n");
        printf("Griglia            : %d x %d\n", NX, NY);
        printf("Processi MPI       : %d\n", nproc);
        printf("Thread OpenMP/proc : %d  (Total core = %d)\n", n_thread, nproc * n_thread);
        printf("alpha              : %.3f\n", alpha);
        printf("Iterazioni fatte   : %d\n", iter_done);
        printf("Errore finale      : %.3e", global_error);
        printf("%s\n", (global_error < TOLERANCE  ) ? "  -> convergito" : "  -> max iterazioni");
        printf("Tempo totale       : %.4f s\n", global_time);
        printf("Tempo calcolo      : %.4f s\n", compute_time);
        printf("Tempo halo exchange: %.4f s  (%.1f%% del totale)\n",
               halo_time, 100.0 * halo_time / global_time);
        printf("Throughput         : %.2f MUPS\n\n", mups);
    /* Salvataggio in un file CSV per l'analisi dello scaling */
    FILE *f = fopen("../PROGETTO_OUTPUT/metriche.csv", "a");
    if (f != NULL) {
        /* Controllo se il file è vuoto per scrivere l'intestazione */
        fseek(f, 0, SEEK_END);
        if (ftell(f) == 0) {
            fprintf(f, "NX,NY,nproc,alpha,nthread,iter_done,global_error,t_tot,t_compute,t_halo,perc_halo,throughput\n");
        }
        double perc_halo = 100.0 * halo_time / global_time;
        fprintf(f, "%d,%d,%d,%.3f,%d,%d,%.3e,%.4f,%.4f,%.4f,%.2f,%.2f\n",
                NX, NY, nproc, alpha, n_thread, iter_done,
                global_error, global_time, compute_time, halo_time, perc_halo, mups);
        fclose(f);
    } else {
        fprintf(stderr, "Error opening 'metriche.csv'\n");
    }
}

int saveDir(){
    struct stat st = {0};
    if (stat("../PROGETTO_OUTPUT", &st) == -1) {
#if defined(_WIN32)
        return mkdir("../PROGETTO_OUTPUT");
#else
        return mkdir("../PROGETTO_OUTPUT", 0777);
#endif
    }
    return 0;
}

int main(int argc, char **argv){
    /*
    // ARCHITETTURE RICHIESTE //
        ● Decomposizione del dominio (MPI) [...] 
    */
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    /*
    // INSERT INPUT //
        ● Dimensioni della griglia NX × NY (es. 512×512, 1024×1024) 
        ● Coefficiente di diffusione α (default: 0.25, garantisce stabilità numerica) 
        ● Numero di iterazioni temporali 
        ● Condizioni iniziali: bordi a temperatura 0, sorgente calda al centro (T=100)
    */

    if (argc < 4) {
        if (rank == 0){
            fprintf(stderr, "Error: Missing arguments!\n"
                            "Usage: %s <rows> <cols> <iterations>\n"
                        "You can also indicate: --diffusion <alpha> --temp-edges <T> --temp-source <T> --save-each<N>\n", argv[0]);
        }
        MPI_Finalize();
        return -1;
    }

    if(rank == 0){
        if(saveDir() != 0){
            fprintf(stderr, "Error: Failed to create output directory\n");
            MPI_Finalize();
            return -1;
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);

    int NX = atoi(argv[1]);
    int NY = atoi(argv[2]);
    int max_iter = atoi(argv[3]);
    double alpha = 0.25;
    int temp_edges = 0;
    int temp_source = 100;
    int save_each = 100;
    if(NX <= 0){
        if(rank == 0) fprintf(stderr, "Error: Number of rows must be positive\n");
        MPI_Finalize();
        return -1;
    }
    if(NY <= 0){
        if(rank == 0) fprintf(stderr, "Error: Number of columns must be positive\n");
        MPI_Finalize();
        return -1;
    }
    if(max_iter <= 0){
        if(rank == 0) fprintf(stderr, "Error: Number of iterations must be positive\n");
        MPI_Finalize();
        return -1;
    }

    static struct option long_options[] = {
        {"diffusion", required_argument, 0, 'a'},
        {"temp-edges", required_argument, 0, 'e'},
        {"temp-source", required_argument, 0, 's'},
        {"save-each", required_argument, 0, 'n'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "a:e:s:n:", long_options, NULL)) != -1) {
        switch (opt) {

            case 'a':
                alpha = atof(optarg);
                // Coefficiente di diffusione α (default: 0.25, garantisce stabilità numerica) //
                if(alpha > 0.25 && rank == 0)
                    printf("Warning: alpha = %.3f > 0.25, the simulation might be unstable!\n", alpha);
                break;

            case 'e':
                temp_edges = atoi(optarg);
                break;

            case 's':
                temp_source = atoi(optarg);
                break;
                
            case 'n':
                save_each = atoi(optarg);
                break;

            default:
                if (rank == 0)
                    fprintf(stderr, "Unknown option\n");
                MPI_Finalize();
                return 1;
        }
    }

    if (temp_source <= temp_edges) {
        if (rank == 0) {
            fprintf(stderr, "Error: Temperature of source must be greater than temperature of edges");
        }
        MPI_Finalize();
        return 1;
    }

    /*
    // ARCHITETTURE RICHIESTE //
        ● [...]  la griglia viene suddivisa in strisce 
          orizzontali, una per processo MPI. Ogni processo gestisce NX/size righe più 
          due righe fantasma (halo). 
    */
    if(NX % size != 0){
        if(rank == 0)
            fprintf(stderr, "Error: Number of rows (%d) is not divisible by number of processes (%d).\n", NX, size);
        MPI_Finalize();
        return 1;
    }
    int local_rows = NX/size;
    int starting_row = rank * local_rows;
    int total_rows = local_rows+2;  
    double *T = (double*) calloc((size_t)total_rows * NY, sizeof(double));
    double *new_T = (double*) calloc((size_t)total_rows * NY, sizeof(double));
    
    if(T == NULL || new_T == NULL){
        fprintf(stderr, "Error: Memory allocation failed for process %d\n", rank);
        free(T);
        free(new_T);
        MPI_Finalize();
        return 1;
    }
  
    int center_row = NX/2;
    int center_col = NY/2;
    int source_width = (NX < NY ? NX : NY) / 20;
    if(source_width < 1) source_width = 1;

    for(int i = 0; i < total_rows; i++){
        int global_row = starting_row + (i-1);
        for(int j = 0; j<NY; j++){
            if(abs(global_row - center_row) <= source_width &&
                abs(j - center_col) <= source_width){
                    T[i*NY+j] = temp_source;
                    new_T[i*NY+j] = temp_source;
                }
            else{
                T[i*NY+j] = temp_edges;
                new_T[i*NY+j] = temp_edges;
            }
        }
    }

    /*
    // ARCHITETTURE RICHIESTE //
        ●  Halo exchange [...]
    */
    int rank_above = (rank > 0) ? (rank - 1) : MPI_PROC_NULL;
    int rank_below = (rank < size - 1) ? (rank + 1) : MPI_PROC_NULL;
    double *global_grid = NULL;
    if(rank == 0){
        global_grid = (double*) malloc((size_t) NX * NY * sizeof(double));
        if(global_grid == NULL){
            fprintf(stderr, "Error: malloc failed for global_grid\n");
            free(T); free(new_T);
            MPI_Finalize();
            return 1;
        }
    }
    double *error_history = NULL;
    if(rank == 0){
        error_history = (double*) malloc((size_t) max_iter * sizeof(double));
        if(error_history == NULL){
            fprintf(stderr, "Error: malloc failed for error_history\n");
            free(T); free(new_T);
            free(global_grid);
            MPI_Finalize();
            return 1;
        }
    }
    double halo_time = 0.0, compute_time = 0.0;
    double global_error = 0.0;
    int iter =0;

    MPI_Barrier(MPI_COMM_WORLD);
    double starting_time = MPI_Wtime();

    /*
    // ARCHITETTURE RICHIESTE //
        ●  Calcolo locale (OpenMP parallel for)[...]
    */
    int n_thread = 1;
    #if defined(_OPENMP)
        #pragma omp parallel
        #pragma omp single
        n_thread = omp_get_num_threads();
    #endif

    for (iter = 1; iter <= max_iter; iter++){
    /*
        // ARCHITETTURE RICHIESTE //
        ● [...] (MPI_Sendrecv): ad ogni iterazione ogni processo scambia 
            le righe di bordo con i vicini prima di applicare lo stencil.
    */
    double t1 = MPI_Wtime();
    MPI_Sendrecv(&T[1*NY], NY, MPI_DOUBLE, rank_above, 0,
             &T[(local_rows+1)*NY], NY, MPI_DOUBLE, rank_below, 0,
             MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Sendrecv(&T[local_rows*NY],NY,MPI_DOUBLE, rank_below, 1,
                &T[0], NY,MPI_DOUBLE, rank_above, 1, 
                MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    halo_time += MPI_Wtime() - t1;
    
    /*
        // ARCHITETTURE RICHIESTE //
        ● [...]  (OpenMP parallel for):  il loop sulle righe del proprio blocco 
            viene parallelizzato con OpenMP. 
        
    */
    double t2 = MPI_Wtime();
    double local_error = 0.0;

    #pragma omp parallel for reduction(max: local_error)
    for(int i = 1; i<= local_rows; i++){
        int glob_row = starting_row +(i-1);
        for(int j = 0; j < NY; j++){
            if (glob_row == 0 || glob_row == NX - 1 || j == 0 || j == NY - 1) {
                new_T[i*NY + j] = temp_edges;
                continue;
            }
            if (abs(glob_row - center_row) <= source_width &&
                abs(j - center_col) <= source_width) {
                new_T[i*NY + j] = temp_source;
                continue;
            }
            /* 
                // BACKGROUND //
                Equazione del calore discreta con differenze esplicite: 
                T[i][j](t+1) = T[i][j](t) + α*dt/dx² * (T[i+1][j] + T[i-1][j] + T[i][j+1] + T[i][j-1] - 4*T[i][j](t))
            */
            double old_temp = T[i*NY + j];
            double new_temp = old_temp + alpha * ( T[(i + 1)*NY + j] + T[(i - 1)*NY + j]
                                             + T[i*NY + j + 1] + T[i*NY + j - 1]
                                             - 4.0 * old_temp );
            new_T[i*NY + j] = new_temp;

            double diff = fabs(new_temp - old_temp);
            if (diff > local_error) local_error = diff;
        }
    }
    compute_time += MPI_Wtime() - t2;
    
    /*
        // ARCHITETTURE RICHIESTE //
        ● Convergenza (MPI_Allreduce): ogni processo calcola l'errore massimo 
            locale max|T_new - T_old|; tramite MPI_Allreduce(MPI_MAX) si 
            verifica se la soluzione è andata a convergenza globalmente.
    */
    MPI_Allreduce(&local_error, &global_error,1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
    
    /*
        // OUTPUT RICHIESTO //
        ● Griglia di temperatura T[i][j] salvata in CSV ogni N iterazioni, 
            visualizzabile con Python/matplotlib 
    */
    if(save_each > 0 && iter % save_each == 0){
        MPI_Gather(&T[1*NY], local_rows*NY, MPI_DOUBLE, global_grid, local_rows*NY, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        if(rank == 0){

            char filename[128];
            sprintf(filename, "../PROGETTO_OUTPUT/calore_%05d.csv", iter);
            save_grid(filename, global_grid, NX, NY);
        }
    }

    double *tmp = T; T = new_T; new_T = tmp;
    if(rank == 0) error_history[iter-1] = global_error;
    if(global_error < TOLERANCE) break;
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double total_time = MPI_Wtime()-starting_time;
    int iter_done = (iter > max_iter) ? max_iter : iter;

    MPI_Gather(&T[1*NY], local_rows*NY, MPI_DOUBLE, 
        global_grid, local_rows*NY, MPI_DOUBLE,
        0, MPI_COMM_WORLD);
    if(rank == 0){
        save_grid("../PROGETTO_OUTPUT/calore_finale.csv", global_grid, NX, NY);
    }

    /*
        // OUTPUT RICHIESTO //
        ● Errore di convergenza in funzione delle iterazioni 
    */
    if(rank == 0){
        FILE *fc = fopen("../PROGETTO_OUTPUT/convergenza.csv", "w");
        if(fc != NULL){
            fprintf(fc, "iter, error\n");
            for(int k = 0; k< iter_done; k++){
                fprintf(fc, "%d, %.8e\n", k+1, error_history[k]);
            }
            fclose(fc);
        }
    }
    
    if(rank == 0){
        save_metrics(NX, NY, size, n_thread, alpha, iter_done,
                     global_error, total_time, compute_time, halo_time);  
    }

    free(T);
    free(new_T);
    if(rank == 0){
        free(global_grid);
        free(error_history);
    }
    
    MPI_Finalize();
    return 0;
}