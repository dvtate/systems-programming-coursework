#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <math.h>
#include <inttypes.h>
#include <string.h>

// could use PPC to compile 2 versions of everything...
char flag_verbose = 0;

// simulation on 64-bit machine
#define ADDR_BITS 64

#include "cachelab.h"

#include "util.h"

/*

Usage: ./csim-ref [-hv] -s <s> -E <E> -b <b> -t <tracefile>
-h: Optional help flag that prints usage info
-v: Optional verbose flag that displays trace info
-s <s>: Number of set index bits (S = 2s is the number of sets)
-E <E>: Associativity (number of lines per set)
-b <b>: Number of block bits (B = 2b is the block size)
-t <tracefile>: Name of the valgrind trace to replay

*/

// datatype to simulate cache
struct cache_block_t
{
    int valid;
    int dirty;
    char *tag;
    //char *block;
};

// parsed valgrind line
struct vg_acc_t {

    // request type
    enum vg_acc_type_t {
        VG_INSTR_LOAD = 'I',
        VG_DATA_LOAD  = 'L',
        VG_DATA_STORE = 'S',
        VG_DATA_MOD   = 'M',
    } operator;
    // hex address
    char address[ADDR_BITS / 8 + 1];
    // number of bytes
    unsigned char size;
};


struct cache_t {
    uint64_t nsets;
    uint16_t lines_per_set;
    uint64_t block_size; // in bytes
    uint64_t hits;
    uint64_t misses;
    uint64_t reads;
    uint64_t writes;

    struct cache_block_t* blocks;
};

// return 1 on failure/eof
char parse_line(FILE* trace, struct vg_acc_t* ret)
{
    static size_t line_len = 0;
    static char* line = NULL;

read_line:;
    unsigned int res = getline(&line, &line_len, trace);
    if (res < 0)
        return 0;

    // ignore comments
    if (line[0] == '-' || line[0] == '=')
        goto read_line;

    // mutable copy of line that we can
    char* mline = line;

    // ignore instruction type
    if (mline[0] == 'I') {
        goto read_line;
        ret->operator = VG_INSTR_LOAD;
        mline += 3;
    } else {
        ret->operator = mline[1];
        mline += 3;
    }

    // read address
    strncpy(ret->address, mline, ADDR_BITS / 8);
    ret->address[ADDR_BITS / 8] = '\0';
    mline += ADDR_BITS / 8 + 1;

    // read size
    ret->size = atoi(mline);

    // success
    return 1;
}

// convert hex string to a binary string
void htob(char* in, char* out)
{
    *out = '\0';
    while (*in) {
        switch (*in) {
            case '0': strcat(out, "0000"); break;
            case '1': strcat(out, "0001"); break;
            case '2': strcat(out, "0010"); break;
            case '3': strcat(out, "0011"); break;
            case '4': strcat(out, "0100"); break;
            case '5': strcat(out, "0101"); break;
            case '6': strcat(out, "0110"); break;
            case '7': strcat(out, "0111"); break;
            case '8': strcat(out, "1000"); break;
            case '9': strcat(out, "1001"); break;
            case 'a': case 'A': strcat(out, "1010"); break;
            case 'b': case 'B': strcat(out, "1011"); break;
            case 'c': case 'C': strcat(out, "1100"); break;
            case 'd': case 'D': strcat(out, "1101"); break;
            case 'e': case 'E': strcat(out, "1110"); break;
            case 'f': case 'F': strcat(out, "1111"); break;
            default:
                printf("invalid hex: %s\n", in);
                abort();
                return;
        }

        in++;
    }
}

// parse hex str into int
uint64_t htoi(char* in)
{
    unsigned int ret = 0;
    while (*in) {
        switch (*in) {
            case '0': break;
            case '1': ret += 1; break;
            case '2': ret += 2; break;
            case '3': ret += 3; break;
            case '4': ret += 4; break;
            case '5': ret += 5; break;
            case '6': ret += 6; break;
            case '7': ret += 7; break;
            case '8': ret += 8; break;
            case '9': ret += 9; break;
            case 'a': ret += 10; break;
            case 'b': ret += 11; break;
            case 'c': ret += 12; break;
            case 'd': ret += 13; break;
            case 'e': ret += 14; break;
            case 'f': ret += 15; break;
            default: printf("invalid hex string: %s\n", in);
        }
        in++;
        ret <<= 4;
    }
    return ret >> 4;
}

void itoh(uint64_t n, char* ret)
{
    sprintf(ret, "%lx", n);
}

/* btoi
 * Converts a binary string to an integer. Returns 0 on error.
 * src: http://www.daniweb.com/software-development/c/code/216372
 *
 * @param   bin     binary string to convert
 * @result  int     decimal representation of binary string
 */
uint64_t btoi(char *bin)
{
    int  b, k, m, n;
    uint64_t  len, sum;

    sum = 0;
    len = strlen(bin) - 1;

    for(k = 0; k <= len; k++) {
        n = (bin[k] - '0');
        if ((n > 1) || (n < 0))
            return 0;
        for(b = 1, m = len; m > k; m--)
            b *= 2;
        sum = sum + n * b;
    }
    return(sum);
}

void itob(unsigned int num, char* ret)
{
    ret[ADDR_BITS] = '\0';
    for(int i = 0; i < ADDR_BITS; i++)
        ret[ADDR_BITS - 1 - i] = (num == ((1 << i) | num)) ? '1' : '0';
    return;
}

// converts address to a cache lookup
void address_to_lookup(char* addr, char* lookup const int setbits, const int offsetbits) {
    if (flag_verbose) {
        printf("Address:%s\n", addr);
    }


    // copy tag
    strncpy(lookup, addr, ADDR_BITS - )
}

int main(int argc, char** argv)
{

    int c;

    int nsets = 0, lines_per_set = 0, block_size = 0;
    char* trace_file = NULL;
    int setbits = 0, offsetbits = 0;

    // read command line args
    while ((c = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch (c) {
            case 'h':
                printf("Usage: ./csim-ref [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n"
                "\t-h: Optional help flag that prints usage info\n"
                "\t-v: Optional verbose flag that displays trace info\n"
                "\t-s <s>: Number of set index bits (S = 2^s is the number of sets)\n"
                "\t-E <E>: Associativity (number of lines per set)\n"
                "\t-b <b>: Number of block offset bits (B = 2^b is the block size)\n"
                "\t-t <tracefile>: Name of the valgrind trace to replay)\n");
                return 0;
            case 'v':
                flag_verbose = 1;
                break;
            case 's':
                setbits = atoi(optarg);
                nsets = pow(2, setbits);
                break;
            case 'E':
                lines_per_set = atoi(optarg);
                break;
            case 'b':
                offsetbits = atoi(optarg);
                block_size = pow(2, offsetbits);
                break;
            case 't':
                trace_file = optarg;
                break;
            default:
                abort();
        }
    }

    // printf("%d %d %d %p\n",nsets, lines_per_set, block_size, trace_file );

    // missing/invalid args
    if (setbits < 1)
        return fprintf(stderr, "Missing/invalid -s option\n");
    if (lines_per_set < 1)
        return fprintf(stderr, "Missing/invalid -E option\n");
    if (offsetbits < 1)
        return fprintf(stderr, "Missing/invalid -b option\n");
    if (trace_file == NULL)
        return fprintf(stderr, "Missing/invalid -t option\n");

    FILE* f = fopen(trace_file, "wt");
    if (!f)
        return fprintf(stderr, "could not open file %s", trace_file);

    const unsigned int tagsize = ADDR_BITS - offsetbits - setbits;

    // initialize cache
    struct cache_t cache = { .nsets=nsets,  .lines_per_set = lines_per_set, .block_size=block_size };

    // initialize cache blocks
    cache.blocks = (struct cache_block_t*) malloc(sizeof(struct cache_block_t) * nsets * lines_per_set);
    for (unsigned int i = 0; i < nsets * lines_per_set; i++) {
        cache.blocks[i].tag = (char*) malloc(sizeof(char) * (tagsize + 1));

        for (unsigned int j = 0; j < tagsize; j++)
            cache.blocks[i].tag[j] = '0';

        cache.blocks[i].valid = 0;
        cache.blocks[i].dirty = 0;
    }

    char tag[ADDR_BITS];
    char index[ADDR_BITS];
    char offset[ADDR_BITS];
    char addr[ADDR_BITS + 1];

    struct vg_acc_t cmd;
    while (!parse_line(f, &cmd)) {
        htob(cmd.address, addr);
        printf("addr:%s", addr);
    }


    return 0;
}
