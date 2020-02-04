#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int main(int argc, char** argv) {
	if (argc < 3) {
		printf("Usage: %s STRING1 STRING2\n", argv[0]);
		printf("       %s -d STRING\n", argv[0]);
		exit(1);
	}
	
	const char* from = argv[1];
	const unsigned int terms = strlen(from);

	const char* to = argv[2];

	if (strlen(to) != strlen(from)) {
		printf("STRING1 and STRING2 must have the same length\n");
		exit(1);
	}
	
	char c;
	while ((c = fgetc(stdin)) != EOF) {
		for (unsigned int i = 0; i < terms; i++)
			if (from[i] == c) {
				c = to[i];
				break;
			}

		fputc(c, stdout);
	}


	return 0;
}
