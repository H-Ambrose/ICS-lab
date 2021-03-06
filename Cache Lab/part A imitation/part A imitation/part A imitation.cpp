/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include <stdio.h>
#include "cachelab.h"
#include "contracts.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. The REQUIRES and ENSURES from 15-122 are included
 *     for your convenience. They can be removed if you like.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
	REQUIRES(M > 0);
	REQUIRES(N > 0);

	ENSURES(is_transpose(M, N, A, B));
}


char transpose_submit_32_32_desc[] = "Transpose submission_32_32";
void transpose_submit_32_32(int M, int N, int A[N][M], int B[M][N])
{
	int i, j, k, q;
	int x0, x1, x2, x3, x4, x5, x6, x7;
	if (M == 32) {
		for (k = 0;k < N;k += 8)
			for (q = 0;q < M;q += 8)
				for (i = k;i < k + 8;++i) {
					if (k == q) {
						x0 = A[i][q], x1 = A[i][q + 1];
						x2 = A[i][q + 2], x3 = A[i][q + 3];
						x4 = A[i][q + 4], x5 = A[i][q + 5];
						x6 = A[i][q + 6], x7 = A[i][q + 7];

						B[q][i] = x0, B[q + 1][i] = x1;
						B[q + 2][i] = x2, B[q + 3][i] = x3;
						B[q + 4][i] = x4, B[q + 5][i] = x5;
						B[q + 6][i] = x6, B[q + 7][i] = x7;
					}
					else
						for (j = q;j < q + 8;++j)
							B[j][i] = A[i][j];
				}
	}
}

char transpose_submit_64_64_desc[] = "Transpose submission_64_64";
void transpose_submit_64_64(int M, int N, int A[N][M], int B[M][N])
{
	int i, j, k, q;
	int x0, x1, x2, x3, x4, x5, x6, x7;
	if (M == 64) {
		for (k = 0;k < N;k += 8)
			for (q = 0;q < M;q += 8) {
				for (i = k;i < k + 4;++i) {
					x0 = A[i][q], x1 = A[i][q + 1];
					x2 = A[i][q + 2], x3 = A[i][q + 3];
					x4 = A[i][q + 4], x5 = A[i][q + 5];
					x6 = A[i][q + 6], x7 = A[i][q + 7];

					B[q][i] = x0, B[q + 1][i] = x1;
					B[q + 2][i] = x2, B[q + 3][i] = x3;
					B[q][i + 4] = x4, B[q + 1][i + 4] = x5;
					B[q + 2][i + 4] = x6, B[q + 3][i + 4] = x7;
				}
				for (j = q;j < q + 4;++j) {
					x0 = A[i][j], x1 = A[i+1][j];
					x2 = A[i+2][j], x3 = A[i+3][j];

					x4 = B[j][i], x5 = B[j][i + 1];
					x6 = B[j][i + 2], x7 = B[j][i + 3];

					B[j][i] = x0, B[j][i + 1] = x1;
					B[j][i + 2] = x2, B[j][i + 3] = x3;

					B[j + 4][k] = x4, B[j + 4][k + 1] = x5;
					B[j + 4][k + 2] = x6, B[j + 4][k + 3] = x7;
				}
				//till now the bottom part of the 8*8 array block are all loaded
				for (;i < k + 8;++i) {
					x0 = A[i][j], x1 = A[i][j + 1];
					x2 = A[i][j + 2], x3 = A[i][j + 3];

					B[j][i] = x0, B[j + 1][i] = x1;
					B[j + 2][i] = x2, B[j + 3][i] = x3;
				}

			}
	}
}

char transpose_submit_61_67_desc[] = "Transpose submission_61_67";
void transpose_submit_61_67(int M, int N, int A[N][M], int B[M][N])
{
	int i, j, k, q;
	for (k = 0;k < N;k += 17) 
		for (q = 0;q < M;q += 17) 
			for (i = k;i < N&&i < k + 17;i++) 
				for (j = q;j < M&&j < q + 17;j++) 
					B[j][i] = A[i][j];
}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */

 /*
  * trans - A simple baseline transpose function, not optimized for the cache.
  */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
	int i, j, tmp;

	REQUIRES(M > 0);
	REQUIRES(N > 0);

	for (i = 0; i < N; i++) {
		for (j = 0; j < M; j++) {
			tmp = A[i][j];
			B[j][i] = tmp;
		}
	}

	ENSURES(is_transpose(M, N, A, B));
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
	/* Register your solution function */
	registerTransFunction(transpose_submit, transpose_submit_desc);
	registerTransFunction(transpose_submit_32_32, transpose_submit_32_32_desc);
	registerTransFunction(transpose_submit_64_64, transpose_submit_64_64_desc);
	registerTransFunction(transpose_submit_61_67, transpose_submit_61_67_desc);

	/* Register any additional transpose functions */
	registerTransFunction(trans, trans_desc);

}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
	int i, j;

	for (i = 0; i < N; i++) {
		for (j = 0; j < M; ++j) {
			if (A[i][j] != B[j][i]) {
				return 0;
			}
		}
	}
	return 1;
}
