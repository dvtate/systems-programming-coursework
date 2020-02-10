#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int main(int argc, char** argv) {
	if (argc < 3) {
		printf("Usage: %s STRING1 STRING2\n", argv[0]);
		printf("       %s -d STRING\n", argv[0]);
		exit(1);
	}


	char delete_mode;
	char* from = argv[1];
	char* to = argv[2];

	if (strcmp(from, "-d") == 0) {
		from = to;
		delete_mode = 1;
	} else if (strcmp(to, "-d") == 0) {
		delete_mode = 1;
	} else {
		delete_mode = 0;
	}

	const unsigned int terms = strlen(from);

	if (strlen(to) != strlen(from)) {
		printf("STRING1 and STRING2 must have the same length\n");
		exit(1);
	}

	char c;
	unsigned int i;
	while ((c = fgetc(stdin)) != EOF) {
		for (i = 0; i < terms; i++)
			if (from[i] == c) {
				c = to[i];
				if (!delete_mode)
					fputc(c, stdout);
				break;
			}
		if (i == terms) 
			fputc(c, stdout);
	}


	return 0;
}

