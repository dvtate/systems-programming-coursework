#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>


int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Usage: %s FILENAME\n", argv[0]);
		exit(1);
	}

	FILE* f = fopen(argv[1], "r");
	if (!f) {
		fprintf(stderr, "ERROR: File not found: %s", argv[1]);
		exit(1);
	}

	//FILE* f = stdin;
	uint32_t dups = 1;
	char prev = fgetc(f);
	char c;

	while (1) {
		c = fgetc(f);
		if (c == prev) {
			dups++;
		} else {
			fwrite(&dups, sizeof(dups), 1, stdout);
			fputc(prev, stdout);
			//printf("%d%c", dups, prev);
			dups = 1;
			prev = c;
		}

		if (c == EOF)
			break;
	}

	//fflush(stdout);
	fclose(f);

	return 0;
}
