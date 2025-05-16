#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {

	if(argc < 2){
		printf("wunzip: file1 [file2 ...]\n");
		exit(1);
	}

	for(int i = 1; i < argc; i++) {

		FILE *fp = fopen(argv[i], "r");

		if (fp == NULL) {
			printf("wunzip: cannot open file\n");
			exit(1);
		}

		int count;
		char c;

		while(fread(&count, sizeof(int), 1, fp)) {
			fread(&c, sizeof(char), 1, fp);
			while(count--) {
				fwrite(&c, sizeof(char), 1, stdout);
			}
		}
	}

	return 0;
}
