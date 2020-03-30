/*
 * cachelab.c - Cache Lab helper functions
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "cachelab.h"
#include <time.h>

trans_func_t func_list[MAX_TRANS_FUNCS];
int func_counter = 0;

/*
 * printSummary - Summarize the cache simulation statistics. Student cache simulators
 *                must call this function in order to be properly autograded.
 */
void printSummary(int hits, int misses, int evictions)
{
    printf("hits:%d misses:%d evictions:%d\n", hits, misses, evictions);
    FILE* output_fp = fopen(".csim_results", "w");
    assert(output_fp);
    fprintf(output_fp, "%d %d %d\n", hits, misses, evictions);
    fclose(output_fp);
}

/*
 * initMatrix - Initialize the given matrix
 */
void initMatrix(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;
    srand(time(NULL));
    for (i = 0; i < N; i++){
        for (j = 0; j < M; j++){
            // A[i][j] = i+j;  /* The matrix created this way is symmetric */
            A[i][j]=rand();
            B[j][i]=rand();
        }
    }
}

void randMatrix(int M, int N, int A[N][M]) {
    int i, j;
    srand(time(NULL));
    for (i = 0; i < N; i++){
        for (j = 0; j < M; j++){
            // A[i][j] = i+j;  /* The matrix created this way is symmetric */
            A[i][j]=rand();
        }
    }
}

/*
 * correctTrans - baseline transpose function used to evaluate correctness
 */
void correctTrans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;
    for (i = 0; i < N; i++){
        for (j = 0; j < M; j++){
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}



/*
 * registerTransFunction - Add the given trans function into your list
 *     of functions to be tested
 */
void registerTransFunction(void (*trans)(int M, int N, int[N][M], int[M][N]),
                           char* desc)
{
    func_list[func_counter].func_ptr = trans;
    func_list[func_counter].description = desc;
    func_list[func_counter].correct = 0;
    func_list[func_counter].num_hits = 0;
    func_list[func_counter].num_misses = 0;
    func_list[func_counter].num_evictions =0;
    func_counter++;
}

struct TransposeInfo {
    unsigned long long int A;
    unsigned long long int B;
    int M;
    int N;
};

static int transposeInfoStatus = 0;
static struct TransposeInfo transposeInfo;

void initializeArrayAccessConverter(const char *tracefileName) {
    char filename[4096];
    sprintf(filename, "%s.info", tracefileName);
    FILE *file = fopen(filename, "r");
    if (file) {
        int scanned = fscanf(file, "%llx %llx %d %d", &transposeInfo.A, &transposeInfo.B, &transposeInfo.M, &transposeInfo.N);
        fclose(file);
        if (scanned == 4) {
            transposeInfoStatus = 1;
            return;
        }
    }
    transposeInfoStatus = -1;
}

static int internalAddressToArrayAccess(char *outputString, unsigned long long int address, const struct TransposeInfo *transposeInfo) {
    if (transposeInfo->N > 0 && transposeInfo->M > 0) {
        int totalSize = transposeInfo->N * transposeInfo->M;
        if (address >= transposeInfo->A && address < transposeInfo->A + totalSize * sizeof(int)) {
            int offset = (int)((address - transposeInfo->A) / sizeof(int));
            int vPos = offset / transposeInfo->M;
            int hPos = offset % transposeInfo->M;
            return sprintf(outputString, "A[%d][%d]", vPos, hPos);
        }
        if (address >= transposeInfo->B && address < transposeInfo->B + totalSize * sizeof(int)) {
            int offset = (int)((address - transposeInfo->B) / sizeof(int));
            int vPos = offset / transposeInfo->N;
            int hPos = offset % transposeInfo->N;
            return sprintf(outputString, "B[%d][%d]", vPos, hPos);
        }
    }
    return sprintf(outputString, "%llx", address);
}

int addressToArrayAccess(char *outputString, unsigned long long int address) {
    if (transposeInfoStatus == 0) {
        fprintf(stderr, "You must call initializeArrayAccessConverter with the input tracefile's name before using addressToArrayAccess!");
        abort();
    }
    if (transposeInfoStatus < 0) {
        return sprintf(outputString, "%llx", address);
    }
    return internalAddressToArrayAccess(outputString, address, &transposeInfo);
}
