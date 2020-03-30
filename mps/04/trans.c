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

int is_transpose(int M, int N, int A[N][M], int B[M][N]);


void transpose_32x32(int M, int N, int A[M][N], int B[N][M])
{
    int i, j, k, l;
    int temp;

    // operating on matrix using quadrants to reduce cache misses
    // 2 1
    // 3 4

    for (i = 0; i < M; i += 8)
        for (j = 0; j < N; j += 8) {
            // A3 -> B1
            for (k = i + 7; k > i + 3 && k < M; k--)
                for (l = j + 3; l >= j && l < N; l--)
                    B[l][k] = A[k][l];

            // A4 -> B2
            for (k = i + 7; k > i + 3 && k < M; k--)
                for (l = j + 4; l <= j + 7 && l < N; l++)
                    B[l - 4][k - 4] = A[k][l];

            // A1 -> B3
            for (k = i; k <= i + 3 && k < M; k++)
                for (l = j + 4; l <= j + 7 && l < N; l++)
                    B[l][k] = A[k][l];

            // A2 -> B4
            for (k = i; k <= i + 3 && k < M; k++)
                for (l = j; l <= j + 3 && l < N; l++)
                    B[l + 4][k + 4] = A[k][l];

            // swap B2 <--> B4
            for (k = i; k <= i + 3 && k < M; k++)
                for (l = j; l <= j + 3 && l < N; l++) {
                    temp = B[k + 4 + j - i][l + 4 + i - j];
                    B[k + 4 + j - i][l + 4 + i - j] = B[k + j - i][l + i - j];
                    B[k + j - i][l +  i - j] = temp;
                }
        }

}


/*The function transposes 61 by 67 matrix by saving the diagonal elements
 *in temp, which ensures that A and B's
 *cache lines are not accessed at the same time, thereby reducing the
 *number of misses. This is because they map to the same set.
 */
/*Having the block size 18 makes it within the  limit of 2000
 *The number was arrived at by trial and error
 *The random nature of the number is what probably helped.
*/
void transpose_61x67(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, k, l;
    int temp;

    const int bsize = 18;
    for (i = 0; i < M; i += bsize)
        for (j = 0; j < N; j += bsize)
            for (k = i; k < i + bsize && k < M; k++) {
                // for whatever reason diagonal elements cause problems
                for (l = j; l < j + bsize && l < N; l++)

                    if (k == l)
                        temp = A[k][k];
                    else
                        B[k][l] = A[l][k];

                if (i == j)
                    B[k][k] = temp;
            }

}

void transpose_asymetric(int M, int N, int A[N][M], int B[M][N], const unsigned short bsize) {
    int i,j,p,q;
    int temp;

    for(i = 0; i < M; i += bsize) {
        for (j = 0; j < N; j += bsize) {
            for (p = i; p < i + bsize && p < M; p++) {
                // Only for diagonal elements, do we have the problem
                for (q = j; q < j + bsize && q < N; q++)

                    if (p == q)
                        temp = A[p][p];
                    else
                        B[p][q] = A[q][p];

                if (i == j)
                    B[p][p] = temp;

            }
        }
    }
}

void transpose_generic(int M, int N, int A[N][M], int B[M][N], const unsigned short block_size) {
    unsigned short i, j, k, l;
    int *ap, *bp;
    unsigned int in, kn;
    for (i = 0; i < M; i += block_size) {
        in = i * N;
        for (j = 0; j < N; j += block_size) {
            ap = A[0] + in + j;
            bp = B[0] + j * M + i;
            for (k = 0; k < block_size; k++) {
                kn = k * N;
                for (l = 0; l < block_size; l++) {
                    bp[l * M + k] = ap[kn + l];
                }
            }
        }
    }
}

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{

    if (M == 32)
        transpose_32x32(M, N, A, B);
    if (M == 61)
        transpose_61x67(M,N,A,B);
    else if (M == 64)
        transpose_generic(M, N, A, B, (1 << 4) / sizeof(int));
    else if (M != N)
        transpose_asymetric(M, N, A, B, (1 << 5) / sizeof(int));


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

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

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
