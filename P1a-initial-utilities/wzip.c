#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {

	if(argc < 2){
		printf("wzip: file1 [file2 ...]\n");
		exit(1);
	}

	int count = 1;
	char c;
	/* EOF used to indicate the end of a file stream. */
	char prev = EOF;
	for(int i = 1;i < argc; i ++) {

		FILE *fp = fopen(argv[i], "r");

		if (fp == NULL) {
			printf("wzip: cannot open file\n");
			exit(1);
		}

		while ((c = fgetc(fp)) != EOF){
			/* first element */
			if (prev == EOF) {
				/* count is already initilized to 1 */    	
				prev = c;
			}
			/* element changed */
			else if (c != prev) {
				fwrite(&count, sizeof(int), 1, stdout);
				fwrite(&prev, sizeof(char), 1, stdout);
				count = 1;
				prev = c;
			}
			else {
				count++;
			}
		}

		fclose(fp);
	}

	/* last data */
	if(prev != EOF) {
		fwrite(&count, sizeof(int), 1, stdout);
		fwrite(&prev, sizeof(char), 1, stdout);
	}
	
	return 0;
}
