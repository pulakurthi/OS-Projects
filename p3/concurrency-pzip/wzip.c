#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <assert.h>
#include <pthread.h>
#include "common_threads.h"

typedef struct {
	char *to_zip;
	int mem_size;
} myarg_t;

typedef struct {
    int *counts;
    char *chars;
    int n;
} myret_t;

void *worker(void *arg) {
    myarg_t *args = (myarg_t *) arg;
    int   *counts = (int*)malloc(args->mem_size * sizeof(int));
    char  *chars  = (char*)malloc(args->mem_size);
    assert(counts && chars);
	int count = 1,n = 0;
	char c;
	/* EOF used to indicate the end of a file stream. */
	char prev = EOF;
	for(int i = 0; i < args->mem_size;i++){
		c = args->to_zip[i];
		/* first element */
		if (prev == EOF) {
			/* count is already initilized to 1 */    	
			prev = c;
		}
		/* element changed */
		else if (c != prev) {
            counts[n] = count;
            chars[n]  = prev;
            n++;
			count = 1;
			prev = c;
		}
		else {
			count++;
		}
    }
	/* last data */
	if(prev != EOF) {
        counts[n] = count;
        chars[n]  = prev;
        n++;
	}

    myret_t *out = (myret_t *)malloc(sizeof(myret_t));
    out->counts = counts;
    out->chars = chars;
    out->n = n;
    return out;
}

int main(int argc, char *argv[]) {

	if(argc < 2){
		printf("wzip: file1 [file2 ...]\n");
		exit(1);
	}

	// Compute total size of all files
    off_t total_size = 0;
	for(int i = 1;i < argc; i ++) {
        struct stat st;
        if (stat(argv[i], &st) == -1) {
            exit(1);
        }
        total_size += st.st_size;
	}

	// Create one big mmap
	char *all_data = mmap(NULL, total_size, PROT_READ|PROT_WRITE,
                       MAP_PRIVATE|MAP_ANON, -1, 0);
	if (all_data == MAP_FAILED) {
	  exit(1);
	}

    // Read each file into the mmap’d region
    off_t offset = 0;
	for(int i = 1;i < argc; i ++) {

		FILE *fp = fopen(argv[i], "r");

		if (fp == NULL) {
			printf("wzip: cannot open file\n");
			exit(1);
		}

        int n = fread(all_data + offset, 1, total_size - offset, fp);
        offset += n;
        fclose(fp);
	}

	int num_cpus = (int)sysconf(_SC_NPROCESSORS_ONLN);
    int nthreads = num_cpus > total_size ? (int)total_size : num_cpus;

	pthread_t *p = (pthread_t*)malloc(nthreads * sizeof(pthread_t));
    myarg_t *args = (myarg_t*)malloc(nthreads * sizeof(myarg_t));
    myret_t **rvals = (myret_t **)malloc(nthreads * sizeof(myret_t *));

    // partition the mmap’d buffer
    int chunk = total_size / nthreads;
    for(int i = 0; i < nthreads; i++) {
        args[i].to_zip = all_data + i * chunk;
        args[i].mem_size = (i == nthreads-1) ? total_size - i*chunk : chunk;
        Pthread_create(&p[i], NULL, worker, &args[i]);
    }

    for(int i = 0; i < nthreads; i++) {
        Pthread_join(p[i], (void**)&rvals[i]);
	}

   // merge boundaries and write out
    int prev_count, count;
    char prev_char  = EOF, c;
    for(int i = 0; i < nthreads; i++){
        myret_t *out = rvals[i];
        for(int j = 0; j < out->n; j++){
            count = out->counts[j];
            c   = out->chars[j];
			/* first element of first block/thread */
            if(prev_char == EOF){
                prev_count = count;
                prev_char = c;
			/* merge */
            } else if(c == prev_char){
                prev_count += count;
            } else {
                fwrite(&prev_count,  sizeof(int), 1, stdout);
                fwrite(&prev_char, sizeof(char),1, stdout);
                prev_count  = count;
                prev_char = c;
            }
        }
        free(out->counts);
        free(out->chars);
        free(out);
    }
	/* last data */
    if(prev_char != EOF){
        fwrite(&prev_count,  sizeof(int), 1, stdout);
        fwrite(&prev_char, sizeof(char),1, stdout);
    }
    free(rvals);
    free(args);
    free(p);
    munmap(all_data, total_size);
	return 0;
}
