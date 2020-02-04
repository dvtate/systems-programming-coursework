#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Usage: %s FILENAME\n", argv[0]);
		exit(1);
	}
	
	// rb = read binary
	FILE* f = fopen(argv[1], "rb");
	if (!f) {
		fprintf(stderr, "%s: Error: file not found: %s", argv[0], argv[1]);
		exit(1);
	}

	// this way we can get the counter and char at same time
	struct read_head_t {
		unsigned int n : 32; // number of occurances
		char c : 8; // relevant character
		// also probably some padding at the end
	} rh;
	
	// union 5 bytes into readhead from the file
	// - can't use sizeof read_head_t because of the padding compiler adds
	while (fread(&rh, 5, 1, f) == 1)
		for (; rh.n > 0; rh.n--)
			fputc(rh.c, stdout);
	

	return 0;
}
