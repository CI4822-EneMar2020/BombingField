#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <assert.h>
#include <string.h>

struct Attack {
	// Coordinates X, Y, the size R of the square radius and the power
	int X, Y, R, P;
};
typedef struct Attack Attack;

struct Target {
	// Coordinates X, Y and the power
	int X, Y, P;
};
typedef struct Target Target;

int min(int X, int Y) {
	return (X < Y) ? X : Y;
}

int max(int X, int Y) {
	return (X >= Y) ? X : Y;
}
// Returns true if x is in range [low..high], else false 
int inRange(int low, int high, int x) 
{ 
    return  x >= low && x <= high ? 1 : 0; 
} 


Target* copy_targets(Target* targets, int T){
    Target* targets_copy = (Target*)malloc(T * sizeof(Target));
    assert(targets_copy != NULL);
    for(int i=0; i<T; i++){
        targets_copy[i].X = targets[i].X;
        targets_copy[i].Y = targets[i].Y;
        targets_copy[i].P = targets[i].P;
    }
    return targets_copy;
}

int main(int argc, char* argv[]) {

    int *targets_index;

    int N, 	// size of the fields
		T;  // Number of targets

    FILE *fs;
	char *line = NULL;
    size_t len = 0;
    ssize_t read = 0;
    fs = fopen(argv[1], "r");
	read = getline(&line, &len, fs);
	N = atoi(line);
	read = getline(&line, &len, fs);
	T = atoi(line);
    Target* targets = (Target*)malloc(T * sizeof(Target));
	assert(targets != NULL);
    
    // Create targets
    for(int i = 0; i < T; i++){

		read = getline(&line, &len, fs);
		char *token = strtok(line, " ");
		targets[i].X = atoi(token);
		token = strtok(NULL, " ");
		targets[i].Y = atoi(token);
		token = strtok(NULL, " ");
		targets[i].P = atoi(token);
    }

    // Copy targets
    Target* targets_copy = copy_targets(targets, T);

    read = getline(&line, &len, fs);
	int B = atoi(line);
    Attack* attacks = (Attack*)malloc(B * sizeof(Attack));
	assert(attacks != NULL);

    // Create attacks
    for(int i = 0; i < B; i++){

		read = getline(&line, &len, fs);
		char *token = strtok(line, " ");
		attacks[i].X = atoi(token);
		token = strtok(NULL, " ");
		attacks[i].Y = atoi(token);
        token = strtok(NULL, " ");
		attacks[i].R = atoi(token);
		token = strtok(NULL, " ");
		attacks[i].P = atoi(token);
    }


    MPI_Init( &argc, &argv );

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // elements remaining after division among processes
    int elements_remaining = T % world_size;

    if (world_rank == 0) {

        // targets index
        int total_indexes = 2 * world_size; 
        int total_targets = T;
        int per_process = T / world_size;
        targets_index = (int*)malloc(total_indexes * sizeof(int));
        assert(targets_index != NULL);
        // TODO: itero hasta que se acabe total_indexes. Asigno T / total_targets y T / total_targets + elements_remaining son del root
        int index = 0;
        for (int i=0; i<total_indexes - 1; i++) {
            if(i==0)
            {
                targets_index[i] = index;
                targets_index[i+1] = index + per_process + elements_remaining - 1;
                index = index + per_process + elements_remaining;
            }else{
                targets_index[i] = index;
                targets_index[i+1] = index + per_process - 1;
                index = index + per_process;
            }

/*             if(i==0)
            {
                targets_index[i] = index;
                targets_index[i+1] = index + per_process + elements_remaining - 1;
                index = index + per_process + elements_remaining;
            }
            else
            {
                targets_index[i] = index;
                targets_index[i+1] = index + per_process - 1;
                index = index + per_process;
            } */
/*             printf("Index i %d %d\n", i,targets_index[i]);
            printf("Index i+1 %d %d\n", i+1, targets_index[i+1]); */
            i++;
            }

    }
    // Buffer where the received data should be store => each buffer has indexes of targets 
    int *index_per_process = (int*)malloc(2 * sizeof(int));
    assert(index_per_process != NULL);
    // divide the data among processes as described by sendcounts and displs
    MPI_Scatter(targets_index, 2, MPI_INT, index_per_process, 2, MPI_INT, 0, MPI_COMM_WORLD);

    // Process attacks
    for (int i=index_per_process[0]; i<=index_per_process[1]; i++){
        for (int j=0; j<B; j++){
            int x_min = attacks[j].X - attacks[j].R;
            int x_max = attacks[j].X + attacks[j].R;
            int y_min = attacks[j].Y - attacks[j].R;
            int y_max = attacks[j].Y + attacks[j].R;
            int x_target = targets[i].X;
            int y_target = targets[i].Y;
            if(inRange(x_min, x_max, x_target) && inRange(y_min, y_max, y_target)){
                if(targets[i].P > 0){
                    targets[i].P = max(targets[i].P-attacks[j].P, 0);
                }
                else if(targets[i].P < 0){
                    targets[i].P = min(0, targets[i].P+attacks[j].P);
                }
            }
        }
    }

    // printf("DESPUES DE SCATTER!! %d\n",world_rank);
    for (int i=index_per_process[0]; i<=index_per_process[1]; i++){
        printf("Process %d with target X: %d Y: %d", world_rank, targets[i].X, targets[i].Y);
    } 

    // Process counts results
    
    int MTTD = 0;
    int MTPD = 0;
    int MTNA = 0;
    int CTTD = 0;
    int CTPD = 0;
    int CTNA = 0;

    for (int i=index_per_process[0]; i<=index_per_process[1]; i++){
        if(targets_copy[i].P < 0){

            if(targets[i].P >= 0){
                MTTD++;
            }
            else if(targets[i].P > targets_copy[i].P && targets[i].P < 0){
                MTPD++;
            }
            else if(targets[i].P == targets_copy[i].P){
                MTNA++;
            }
        }
        else if(targets_copy[i].P > 0){
            if(targets[i].P <= 0){
                CTTD++;
            }
            else if(targets[i].P < targets_copy[i].P && targets[i].P > 0){
                CTPD++;
            }
            else if(targets[i].P == targets_copy[i].P){
                CTNA++;
            }
        }
    }

    // Reduce all results

    int finalMTTD;
    int finalMTPD;
    int finalMTNA;
    int finalCTTD;
    int finalCTPD;
    int finalCTNA;

    MPI_Reduce(&MTTD,&finalMTTD,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
    MPI_Reduce(&MTPD,&finalMTPD,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
    MPI_Reduce(&MTNA,&finalMTNA,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
    MPI_Reduce(&CTTD,&finalCTTD,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
    MPI_Reduce(&CTPD,&finalCTPD,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
    MPI_Reduce(&CTNA,&finalCTNA,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);

    // hacemos los calculos finales

    if (world_rank==0)
    {
	printf("Military Targets totally destroyed: %d\nMilitary Targets partially destroyed: %d\nMilitary Targets not affected: %d\nCivilian Targets totally destroyed: %d\nCivilian Targets partially destroyed: %d\nCivilian Targets not affected: %d\n",finalMTTD, finalMTPD,finalMTNA,finalCTTD, finalCTPD, finalCTNA);
    }

    MPI_Finalize();
}
