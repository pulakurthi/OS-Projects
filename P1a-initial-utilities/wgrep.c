#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {

	if(argc < 2){
		printf("wgrep: searchterm [file ...]\n");
	 	exit(1);
	}

	char *search = argv[1];

	if(argc == 2){
		char buffer[BUFFER_SIZE];
		while(fgets(buffer, BUFFER_SIZE, stdin) != NULL){
			if(strstr(buffer, search) != NULL){
				printf("%s",buffer);
			}
		}
	}

	for(int i = 2;i < argc; i ++) {

		FILE *fp = fopen(argv[i], "r");

		if (fp == NULL) {
	    	printf("wgrep: cannot open file\n");
	    	exit(1);
	    }

        char *line = NULL;
        size_t linecap = 0;
        ssize_t linelen;
		while ((linelen = getline(&line, &linecap, fp)) > 0){
			if(strstr(line, search) != NULL){
				printf("%s",line);
			}
		}

	    fclose(fp);
	}
	
    return 0;
}

