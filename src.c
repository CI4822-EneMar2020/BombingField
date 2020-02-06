#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "omp.h"

struct Attack {
	// Coordinates X, Y, the size R of the square radius and the power
	int X, Y, R, P;
};
typedef struct Attack Attack;

/**
Copy elements from a matrix to the destiny matrix;
**/
int** matrix_copy(int** from, int N) {
	int** dest = (int**)malloc(N * sizeof(int*));
	assert(dest != NULL);
	for (int r=0; r<N; r++) {
		dest[r] = (int*)malloc(N * sizeof(int));
		assert( dest[r] != NULL );
		for (int c=0; c<N; c++) {
			dest[r][c] = from[r][c];
		}
	}
	return dest;
}

int min(int X, int Y) {
	return (X < Y) ? X : Y;
}

int max(int X, int Y) {
	return (X >= Y) ? X : Y;
}

int main (int argc, char** argv) {
	// Number of threads
	int num_threads = 3;
	if (argc > 1) {
		num_threads = atoi(argv[1]);
	}
	omp_set_num_threads(num_threads);

	// -------------------------------------------------
	int N, 	// size of the fields
		T;	// Number of targets
	scanf("%d%d", &N, &T);

	// Initialize grid
	int** grid = (int**)malloc(N * sizeof(int*));
	assert(grid != NULL);
	for (int r=0; r<N; r++) {
		grid[r] = (int*)malloc(N * sizeof(int));
		assert( grid[r] != NULL );
	}

	// Read data
	for(int i=0; i<T; i++) {
		int r, c; // row, column
		scanf("%d%d", &r, &c);
		scanf("%d", &grid[r][c]);
	}

	// Read attacks
	int B;	// Number of attacks
	scanf("%d", &B);

	Attack* attacks = (Attack*)malloc(B * sizeof(Attack));
	assert(attacks != NULL);
	for (int i=0; i<B; i++) {
		scanf("%d%d%d%d", 
			&attacks[i].X, &attacks[i].Y, 
			&attacks[i].R, &attacks[i].P);
	}

	//----------------------------------------------------------
	// Algorithm
	int** grid1 = matrix_copy(grid, N);

	#pragma omp parallel for shared(grid)
	for (int i=0; i<B; i++) {
		Attack a = attacks[i];
		for (int r=max(0, a.X - a.R); r<min(a.X + a.R, N); r++) {
			for (int c=max(0, a.Y - a.R); c<min(a.Y + a.R, N); c++) {
				if (grid[r][c] > 0) {
					#pragma omp critical
					grid[r][c] = max(grid[r][c]-a.P, 0);
				} else if (grid[r][c] < 0) {
					#pragma omp critical
					grid[r][c] = min(0, grid[r][c]+a.P);
				}
			}
		}
	}

	// for (int r=0; r<N; r++) {
	// 	for (int c=0; c<N; c++) {
	// 		printf("%d\t", grid[r][c]);
	// 	}
	// 	printf("\n");
	// }

	//----------------------------------------------------------
	// Answer
	int destroyed_MTs = 0, 	// Totally destroyed Military Targets
		partially_MTs = 0, 	// Partially destroyed MTs
		not_MTs = 0,	 	// Not affected MTs

		destroyed_CTs = 0,	// Totally destroyed Civilian Targets
		partially_CTs = 0,	// Partially destroyed CTs
		not_CTs = 0;		// Not affected CTs

	#pragma omp parallel for reduction(+:destroyed_MTs)
	for (int r=0; r<N; r++) {
		for (int c=0; c<N; c++) {
			// if it's a MT
			if (grid1[r][c] < 0) {
				if (grid[r][c] >= 0) {
					destroyed_MTs++;
				}
			}
		}
	}
	#pragma omp parallel for reduction(+:partially_MTs)
	for (int r=0; r<N; r++) {
		for (int c=0; c<N; c++) {
			// if it's a MT
			if (grid1[r][c] < 0) {
				if (grid[r][c] > grid1[r][c] && grid[r][c] < 0) {
					partially_MTs++;
				}
			}
		}
	}
	#pragma omp parallel for reduction(+:not_MTs)
	for (int r=0; r<N; r++) {
		for (int c=0; c<N; c++) {
			// if it's a MT
			if (grid1[r][c] < 0) {
				if (grid1[r][c] == grid[r][c]) {
					not_MTs++;
				}
			}
		}
	}
	#pragma omp parallel for reduction(+:destroyed_CTs)
	for (int r=0; r<N; r++) {
		for (int c=0; c<N; c++) {
			// If it's a CT
			if (grid1[r][c] > 0) {
				if (grid[r][c] <= 0) {
					destroyed_CTs++;
				}
			}
		}
	}
	#pragma omp parallel for reduction(+:partially_CTs)
	for (int r=0; r<N; r++) {
		for (int c=0; c<N; c++) {
			// If it's a CT
			if (grid1[r][c] > 0) {
				if (grid[r][c] < grid1[r][c] && grid[r][c] > 0) {
					partially_CTs++;
				}
			}
		}
	}
	#pragma omp parallel for reduction(+:not_CTs)
	for (int r=0; r<N; r++) {
		for (int c=0; c<N; c++) {
			if (grid1[r][c] > 0) {
				if (grid1[r][c] == grid[r][c]){
					not_CTs++;
				}
			}
		}
	}

	printf("Military Targets totally destroyed: %d\n", destroyed_MTs);
	printf("Military Targets partially destroyed: %d\n", partially_MTs);
	printf("Military Targets not affected: %d\n", not_MTs);
	printf("Civilian Targets totally destroyed: %d\n", destroyed_CTs);
	printf("Civilian Targets partially destroyed: %d\n", partially_CTs);
	printf("Civilian Targets not affected: %d\n", not_CTs);

	//----------------------------------------------------------
	// Free memory
	for (int r=0; r<N; r++) {
		free(grid[r]);
	}
	free(grid);
	free(attacks);
}
