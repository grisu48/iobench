/* =============================================================================
 * 
 * Title:         
 * Author:        Felix Niederwanger
 * License:       Copyright (c), 2018 Felix Niederwanger
 *                MIT license (http://opensource.org/licenses/MIT)
 * 
 *  compile:  gcc -Wall -Wextra -pedantic -std=c99 -o iobench iobench.c
 * 
 * =============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>



static const char* test_file = ".test.tmp.out";

static void cleanup() {
	unlink(test_file);
}
long long system_ms() {
    struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = (te.tv_sec*1000LL)*1000L + te.tv_usec;
    return milliseconds;
}

static ssize_t read_buf(void* buf, size_t size) {
	int fd = open("/dev/urandom", O_RDONLY);
	if(fd < 0) return -1;
	ssize_t ret = read(fd, buf, size);
	close(fd);
	return ret;
}

int main(int argc, char** argv) {
    size_t bs = 512;		// Block size
    int iterations = 100;
    int oflags = O_RDWR|O_CREAT;
    long long runtime = 0LL;
    bool verbose = false;
    bool sync_io = false;
    
    if(argc <= 1) {
    	printf("File IO benchmarking utility\n");
    	printf("  Usage: %s FILE [BS] [ITERATIONS] [OPTIONS]\n\n", argv[0]);
    	printf("  BS is the block size in bytes\n");
    	printf("  ITERATIONS the number of iterations\n");
    	printf("  OPTIONS must be after BS and ITERATIONS and can be:\n");
    	printf("  -v  --verbose          Verbose output\n");
    	printf("  -s  --sync             Synchronous IO\n");
    	return EXIT_SUCCESS;
    }
    if(argc > 1) test_file = argv[1];
    if(argc > 2) bs = (size_t)atol(argv[2]);
    if(argc > 3) iterations = atoi(argv[3]);
    for(int i=4; i<argc;i++) {		// Remaining options
    	char *arg = argv[i];
    	if(!strcmp("-s", arg) || !strcmp("--sync", arg) || !strcmp("--direct", arg)) {
    		oflags |= O_SYNC;
    		sync_io = true;
		} else if(!strcmp("-v", arg) || !strcmp("--verbose", arg)) {
			verbose = true;
    	} else {
    		fprintf(stderr, "Unknown argument %s\n", arg);
    		return EXIT_FAILURE;
    	}
    }
    
    char* buf = (char*)malloc(sizeof(char)*bs);
    if(read_buf(buf, bs) != (ssize_t)bs) {
    	fprintf(stderr, "Error reading buffer: %s\n", strerror(errno));
    	return EXIT_FAILURE;
    }
    
    atexit(cleanup);
    printf("Running (%d iterations)\n", iterations);
    long t_open = 0, t_write = 0, t_close = 0;
    for(int i=0;i<iterations;i++) {
    	runtime = -system_ms();
    	int fd = open(test_file, oflags, 0666);
    	if(fd < 0) {
    		fprintf(stderr, "Error opening file '%s': %s\n", test_file, strerror(errno));
    		break;
    	}
    	runtime += system_ms();
    	t_open += runtime;
    	if(verbose) printf("  Open: %lld µs\n", runtime);
    	
    	// Write some stuff
    	runtime = -system_ms();
    	if(write(fd, buf, bs) != (ssize_t)bs) {
    		fprintf(stderr, "Error writing to file '%s': %s\n", test_file, strerror(errno));
    		break;
    	}
    	runtime += system_ms();
    	t_write += runtime;
    	if(verbose) printf("  Write: %lld µs\n", runtime);
    	
    	runtime = -system_ms();
    	if(close(fd) != 0) {
    		fprintf(stderr, "Error closing file '%s': %s\n", test_file, strerror(errno));
    		break;
    	}
    	runtime += system_ms();
    	t_close += runtime;
    	if(verbose) printf("  Close: %lld µs\n", runtime);
    }
    free(buf);
    
    printf("Statistics over %i iterations\n\n", iterations);
    printf("  Synchronous IO: %s\n", (sync_io?"yes":"no"));
    t_open /= iterations; t_write /= iterations; t_close /= iterations;
    printf("  Open  : %ld µs\n", t_open);
    printf("  Write : %ld µs\n", t_write);
    printf("  Close : %ld µs\n", t_close);
    
    printf("Bye\n");
    return EXIT_SUCCESS;
}

