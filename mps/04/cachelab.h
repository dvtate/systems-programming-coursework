/*
 * cachelab.h - Prototypes for Cache Lab helper functions
 */

#ifndef CACHELAB_TOOLS_H
#define CACHELAB_TOOLS_H

#define MAX_TRANS_FUNCS 100

typedef struct trans_func{
  void (*func_ptr)(int M,int N,int[N][M],int[M][N]);
  char* description;
  char correct;
  unsigned int num_hits;
  unsigned int num_misses;
  unsigned int num_evictions;
} trans_func_t;

/*
 * printSummary - This function provides a standard way for your cache
 * simulator * to display its final hit and miss statistics
 */
void printSummary(int hits,  /* number of  hits */
				  int misses, /* number of misses */
				  int evictions); /* number of evictions */

/**
 * Sets up the array access converter below
 * Give it the path to the tracefile
 */
void initializeArrayAccessConverter(const char *tracefileName);

/**
 * Converts an address (like 0x006430c0) to an array access (like A[0][0])
 * Returns the number of characters written
 * @note Writes the hex form of the address if it isn't a part of A or B
 * @warning Requires you to run initializeArrayAccessConverter once first
 */
int addressToArrayAccess(char *outputString, /* string to write into */
                         unsigned long long int address /* address to write */ );

/* Fill the matrix with data */
void initMatrix(int M, int N, int A[N][M], int B[M][N]);

/* The baseline trans function that produces correct results. */
void correctTrans(int M, int N, int A[N][M], int B[M][N]);

/* Add the given function to the function list */
void registerTransFunction(
    void (*trans)(int M,int N,int[N][M],int[M][N]), char* desc);

#endif /* CACHELAB_TOOLS_H */
