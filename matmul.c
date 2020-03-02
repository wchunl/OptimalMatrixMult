#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 1024

volatile __uint64_t A[SIZE][SIZE];
volatile __uint64_t B[SIZE][SIZE];
volatile __uint64_t C[SIZE][SIZE]; // result of matmul()
volatile __uint64_t D[SIZE][SIZE]; // result of matmulHorizontal()
volatile __uint64_t E[SIZE][SIZE]; // result of matmulTile()

int runTimes; // how many times to run the program
int power; // power of tiling. tileSize = [2^1....2^power]

void init(volatile __uint64_t A[][SIZE], volatile __uint64_t B[][SIZE]) {
	int r, c;

	for (c = 0; c < SIZE; c++) {
		for (r = 0; r < SIZE; r++) {
			A[r][c] = rand() % 10;
			B[r][c] = rand() % 10;
		}
	}

}

int verify(volatile __uint64_t C[][SIZE], volatile __uint64_t D[][SIZE]) {
	int r, c;

	for (c = 0; c < SIZE; c++) {
		for (r = 0; r < SIZE; r++) {
			if (C[r][c] != D [r][c]) {
				printf("error!\n");
				return -1;
			}
		}
	}
	return 0;
}

// #1 - Normal matrix multiplication of A and B
void matmul(volatile __uint64_t A[][SIZE], volatile __uint64_t B[][SIZE]) {
	int rowA, colB, idx;

	for (rowA = 0; rowA < SIZE; rowA++) {
		for (colB = 0; colB < SIZE; colB++) {
			for (idx = 0; idx < SIZE; idx++) {
				C[rowA][colB] += A[rowA][idx] * B[idx][colB];
			}
		}
	}
}

// #2 - Horizontal matrix multiplcation of A and B^T
// 		result is put in matrix D
void matmulHorizontal(volatile __uint64_t A[][SIZE], volatile __uint64_t B[][SIZE]) {
	int rowA, rowB, idx;

	for (rowA = 0; rowA < SIZE; rowA++) {
		for (rowB = 0; rowB < SIZE; rowB++) {
			for (idx = 0; idx < SIZE; idx++) {
				// same as matmul(), but multiply across rows of B instead of columns
				D[rowA][rowB] += A[rowA][idx] * B[rowB][idx];
			}
		}
	}
}

// #3 - Horizontal matrix multiplication with tiling optimization
// 		result is put in matrix E
// Source: https://en.wikipedia.org/wiki/Loop_nest_optimization
void matmulTile(volatile __uint64_t A[][SIZE], volatile __uint64_t B[][SIZE], int tileSize) {
	int rowA, rowB, idx;

	for (rowA = 0; rowA < SIZE; rowA += tileSize) { // rows of A
		for (rowB = 0; rowB < SIZE; rowB += tileSize) { // rows of B
			for (idx = 0; idx < SIZE; idx++) { // index
				for(int blk_i = 0; blk_i < tileSize; blk_i++) { // outer tile boundary
					for(int blk_j = 0; blk_j < tileSize; blk_j++) { // inner tile bounday
						E[rowA + blk_i][rowB + blk_j] += A[rowA + blk_i][idx] * B[rowB + blk_j][idx];
					}
				}
			}
		}
	}
}

// transposes matrix B for use in matmulHorizontal() and matmulTile()
// Source: https://en.wikipedia.org/wiki/In-place_matrix_transposition
void transposeB() {
	int row, col, temp;

	for (row = 0; row < SIZE; row++) {
		for (col = 0; col < row; col++) {
			// in-place transpose
			temp = B[row][col];
			B[row][col] = B[col][row];
			B[col][row] = temp;
		}
	}
}


int main(int argc, char **argv) {
	clock_t t;
	double time_taken;

	// Resulting times to calculate average
	double resMul = 0, resHoriz = 0;
	double resTiles[11];

	init(A, B); // Initialize matrices

	// Get runTimes and power from program args
	if (argc == 3) {
		runTimes = strtol(argv[1], NULL, 10);
		power = strtol(argv[2], NULL, 10);
	} else {
		printf("Incorrect number of arguments. Usage: [runTimes] [power]\n");
		exit(-1);
	}

	printf("\n\t-- Running tests %d times, with tile power %d (%d tiles) --\n\n", runTimes, power, (int)pow(2,power));

	for (int avg = 0; avg < runTimes; avg++) { 	// Run each test runTimes times
	// #1 - Matrix multiply without transposition
		printf("Running matmul()\t\t...(%d)", avg+1); fflush(stdout);
		memset((__uint64_t**)C, 0, sizeof(__uint64_t) * SIZE * SIZE);

		t = clock();
		matmul(A, B);
		t = clock() - t;
		time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds

		printf("\ttime: %fs\n", time_taken);
		resMul += time_taken; // record result time for average

	// #2 - Matrix multiply with transposition
		printf("Running matmulHorizontal()\t...(%d)", avg+1); fflush(stdout);
		memset((__uint64_t**)D, 0, sizeof(__uint64_t) * SIZE * SIZE);

		transposeB(); // transpose matrix B

		t = clock();
		matmulHorizontal(A, B); // matrix multiply (A * B^T)
		t = clock() - t;
		time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds

		verify(C,D); // check if result matrices are correct

		printf("\ttime: %fs\n", time_taken);
		resHoriz += time_taken; // record result time for average

	// #3 - Matrix multiply with tiling optimization
		printf("Running matmulTile()\t\t...(%d)\n", avg+1);

		for (int i = 0; i <= power; i++) {
			printf("\t| Running matmulTile() with tile size: %d", (int)pow(2,i)); fflush(stdout);
			memset((__uint64_t**)E, 0, sizeof(__uint64_t) * SIZE * SIZE);

			t = clock();
			matmulTile(A, B, (int)pow(2,i)); // matrix multiply (A * B^T) with tiling
			t = clock() - t;
			time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds

			verify(C,E); // check if result matrices are correct

			printf("\ttime: %fs\n", time_taken);
			resTiles[i] += time_taken; // record time for average
		}
	}

	// Print out the average of the result times
	printf("\n\n\t SUMMARY: \n\t --------\n");

	// #1
	printf("[NORMAL]\t\t matmul() average exec time: %f \n", resMul/(double)runTimes);
	// #2
	printf("[TRANSPOSE]\t\t matmulHorizontal() average exec time: %f \n", resHoriz/(double)runTimes);
	// #3
	for (int i = 0; i <= power; i++) {
		printf("[TILING (%d)]\t\t matmulTile() average exec time: %f \n", (int)pow(2,i), resTiles[i]/(double)runTimes);
	}
}
