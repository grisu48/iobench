/* =============================================================================
 * 
 * Title:         IO Benchmarking utility
 * Author:        Felix Niederwanger
 * License:       Copyright (c), 2018 Felix Niederwanger
 *                MIT license (http://opensource.org/licenses/MIT)
 * 
 * =============================================================================
 */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/wait.h>

#define MAX_CHILDREN 2048

// Max 2048 children possible
static pid_t children[MAX_CHILDREN];
static int nproc = 8;				// Number of parallel processes

/** Get current microseconds for benchmarking */
long long system_us() {
    struct timeval te; 
    gettimeofday(&te, NULL);

    // Drity hack: Subtract seconds sind startup for not overflowing accuracy
    static long p_sec = 0;
    if(p_sec == 0)
    	p_sec = te.tv_sec;
    long seconds = (te.tv_sec-p_sec);

    long long microseconds = (seconds*1000LL*1000LL) + te.tv_usec;
    return microseconds;
}

/** Fill the given buffer with size bytes of random data from /dev/urandom */
static ssize_t fill_buf(void* buf, size_t size) {
	int fd = open("/dev/urandom", O_RDONLY);
	if(fd < 0) return -1;
	ssize_t ret = read(fd, buf, size);
	close(fd);
	return ret;
}

/** Child stats */
struct {
	long t_init;
	long t_open;
	long t_write;
	long t_close;
} typedef cstats_t;

static inline long min(const long x1, const long x2) { return (x1<x2)?x1:x2; }
static inline long max(const long x1, const long x2) { return (x1>x2)?x1:x2; }

static inline void print_stat(cstats_t *stat) {
	printf("%8ld %8ld %8ld %8ld\n", stat->t_init, stat->t_open, stat->t_write, stat->t_close);
}

static void kill_children() {
	for(int i=0;i<nproc;i++) {
		if(children[i] > 0)
			kill(children[i], SIGTERM);
		children[i] = 0;
	}
}

static void sig_handler(int signo) {
	switch(signo) {
		case SIGTERM:
		case SIGINT:
			printf("Termination request\n");
			exit(EXIT_FAILURE);
			break;
		case SIGCHLD:
			// Check if normal termination
			for(int i=0;i<nproc;i++) {
				if(children[i] > 0) {
					int status;
					waitpid(children[i], &status, WNOHANG);
					if(WIFEXITED(status)) {
						children[i] = 0;
						if(WEXITSTATUS(status) != EXIT_SUCCESS) {
							fprintf(stderr, "Abnormal termination of child. Exiting\n");
							kill_children();
							exit(EXIT_FAILURE);
						} else {
							//printf("Child %d exited normally\n", children[i]);
						}
					}
				}
			}
	}
}


int main(int argc, char** argv) {
    size_t bs = 512;			// Write block size
    int count = 10;				// Number of subsequent writes in the file
    char test_dir[1024];		// Test directory
    bzero(test_dir, 1024);
    strcpy(test_dir, ".");
    // Flags for file
    int oflags = O_RDWR|O_CREAT;

	// Parse program arguments
	{
		int c;
		opterr = 0;
		while ((c = getopt (argc, argv, "b:c:C:f:h")) != -1) {
			switch(c) {
				case 'h':
					printf("iobench2 - I/O Benchmarking utility\n");
					printf(" Usage: %s [OPTIONS]\n", argv[0]);
					printf("OPTIONS:\n");
					printf(" -b BYTES          Set block size\n");
					printf(" -c COUNT          Set block numbers to write\n");
					printf(" -C CHILDREN       Set number of children that work in parallel\n");
					printf(" -f DIR            Directory prefix for test files\n");
					exit(EXIT_SUCCESS);
					break;
				case 'b':
					bs = atol(optarg);
					break;
				case 'c':
					count = atoi(optarg);
					break;
				case 'C':
					nproc = atoi(optarg);
					break;
				case 'f':
					strcpy(test_dir, optarg);
					break;
			}
		}
	}



    if(nproc > MAX_CHILDREN) {
    	fprintf(stderr, "Too many children. Max %d possible.\n", MAX_CHILDREN);
    	exit(EXIT_FAILURE);
    }

    long runtime = -system_us();

    // Fork children
    int pipes[nproc][2];
    int iWorker = -1;		// Worker id or -1, if parent
    signal(SIGCHLD, sig_handler);
    for(int i=0;i<nproc;i++) {
    	if(pipe(pipes[i]) != 0) {
    		fprintf(stderr, "Error creating pipe: %s\n", strerror(errno));
    		exit(EXIT_FAILURE);
    	}

    	pid_t pid = fork();
    	if(pid == 0) {
    		// Child writes to the pipe
    		close(pipes[i][0]);
    		iWorker = i;
    		break;
    	} else {
    		children[i] = pid;
    		// Parent reads from the pipe
    		close(pipes[i][1]);
    	}
    }

    if(iWorker >= 0) {
    	// Worker process
    	cstats_t stat;
    	char filename[1100];
    	sprintf(filename, "%s/._test%d.tmp", test_dir, iWorker);

	    // Create random data
	    char* buf = (char*)malloc(sizeof(char)*bs);
	    stat.t_init = -system_us();
	    fill_buf(buf,bs);
	    stat.t_init += system_us();

	    stat.t_open = -system_us();
	    int fd = open(filename, oflags, 0600);
	    stat.t_open += system_us();
	    if(fd < 0) {
	    	fprintf(stderr, "Error opening file '%s': %s\n", filename, strerror(errno));
	    	exit(EXIT_FAILURE);
	    }

	    // Now write stuff to files
	    stat.t_write = -system_us();
	    for(int i=0;i<count;i++) {
	    	if(write(fd, buf, bs) < 0) {
	    		fprintf(stderr, "Error writing to file '%s': %s\n", filename, strerror(errno));
	    		exit(EXIT_FAILURE);
	    	}
	    }
	    stat.t_write += system_us();

	    stat.t_close = -system_us();
	    close(fd);
	    stat.t_close += system_us();

	    //printf("WRITE %3d", iWorker); print_stat(&stat);
		write(pipes[iWorker][1], &stat, sizeof(stat));
	    free(buf);
	    unlink(filename);
	    exit(EXIT_SUCCESS);
    } else {
    	// Parent process waiting for children to exit

	    signal(SIGINT, sig_handler);
	    signal(SIGTERM, sig_handler);
    	
    	cstats_t stat_sum;
    	cstats_t stat_min;
    	cstats_t stat_max;
    	bzero(&stat_sum, sizeof(stat_sum));
    	bzero(&stat_min, sizeof(stat_min));
    	bzero(&stat_max, sizeof(stat_max));
    	printf("                  Init     Open     Write    Close   \n");
	    for(int i=0;i<nproc;i++) {
    		cstats_t stat;
    		ssize_t b_read = 0;
    		while(b_read < (ssize_t)sizeof(stat)) {
	    		ssize_t s = read(pipes[i][0],&stat,sizeof(stat));
	    		if(s < 0) {
	    			if(errno == EINTR) continue;
	    			fprintf(stderr, "Error reading from pipe of child %d: %s\n", i, strerror(errno));
	    			exit(EXIT_FAILURE);
	    		}
	    		b_read += s;
    		}
	    	//printf("READ  %3d", i); print_stat(&stat);
	    	printf("  Child %3d: %8ld %8ld %8ld %8ld\n", i+1, stat.t_init, stat.t_open, stat.t_write, stat.t_close);

	    	if(i == 0) {
	    		stat_min.t_init = stat.t_init;
	    		stat_min.t_open = stat.t_open;
	    		stat_min.t_write = stat.t_write;
	    		stat_min.t_close = stat.t_close;
	    		stat_max.t_init = stat.t_init;
	    		stat_max.t_open = stat.t_open;
	    		stat_max.t_write = stat.t_write;
	    		stat_max.t_close = stat.t_close;
	    	} else {
	    		stat_min.t_init = min(stat_min.t_init, stat.t_init);
	    		stat_min.t_open = min(stat_min.t_open, stat.t_open);
	    		stat_min.t_write = min(stat_min.t_write, stat.t_write);
	    		stat_min.t_close = min(stat_min.t_close, stat.t_close);
	    		stat_max.t_init = max(stat_max.t_init, stat.t_init);
	    		stat_max.t_open = max(stat_max.t_open, stat.t_open);
	    		stat_max.t_write = max(stat_max.t_write, stat.t_write);
	    		stat_max.t_close = max(stat_max.t_close, stat.t_close);
	    	}
	    	stat_sum.t_init += stat.t_init;
	    	stat_sum.t_open += stat.t_open;
	    	stat_sum.t_write += stat.t_write;
	    	stat_sum.t_close += stat.t_close;
	    }

	    runtime = +system_us();

	    printf("\nOverall\n");
	    printf("  Sum      : "); print_stat(&stat_sum);
	    printf("  Min      : "); print_stat(&stat_min);
	    printf("  Max      : "); print_stat(&stat_max);
	    printf("  Average  : %8ld %8ld %8ld %8ld\n", stat_sum.t_init/nproc, stat_sum.t_open/nproc, stat_sum.t_write/nproc, stat_sum.t_close/nproc);

	    long totalBytes = bs * count * nproc;
	    printf("Total: %ld bytes in %ld ms (%0.2f kB/s)\n", totalBytes, runtime/1000L, (float)totalBytes/(runtime/1000.0F*1.024F));


    }


    return EXIT_SUCCESS;
}

