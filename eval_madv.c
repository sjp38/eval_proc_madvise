#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/uio.h>
#include <unistd.h>
#include <time.h>

#define SZ_PAGE (4096)

/* measure time to do madvise() vs process_madvise() */

int measure_madvise(int hint, int sz_mem_to_hint, int sz_single_madvise,
		int measure_batch)
{
	char *buf;
	int i, err, start;
	struct timespec measure_start, measure_end;
	unsigned long elapsed_ns = 0;
	unsigned long nr_measures = 0;

	buf = mmap(NULL, sz_mem_to_hint, PROT_READ | PROT_WRITE, MAP_PRIVATE |
			MAP_ANON, -1, 0);
	if (buf == MAP_FAILED) {
		perror("mmap fail");
		return -1;
	}

	while (elapsed_ns < 5ul * 1000 * 1000 * 1000) {
		memset(buf, 1, sz_mem_to_hint);

		err = clock_gettime(CLOCK_MONOTONIC, &measure_start);
		if (err) {
			perror("clock_gettime() failed\n");
			goto out;
		}
		for (start = 0; start < sz_mem_to_hint;
				start += sz_single_madvise) {
			err = madvise(&buf[start], sz_single_madvise, hint);
			if (err) {
				perror("madvise fail");
				goto out;
			}
		}
		err = clock_gettime(CLOCK_MONOTONIC, &measure_end);
		if (err) {
			perror("clock_gettime() failed\n");
			goto out;
		}
		elapsed_ns += measure_end.tv_sec * 1000000000 +
			measure_end.tv_nsec
			- measure_start.tv_sec * 1000000000 -
			measure_start.tv_nsec;
		nr_measures++;
	}
	printf("%lu\n", elapsed_ns / nr_measures);

out:
	err = munmap(buf, sz_mem_to_hint);
	if (err)
		perror("munamap fail");
	return err;
}

void usage(char *cmd)
{
	printf("usage: %s <hint> <sz_mem> <sz_madv> <sz_p_madv> <nr_measures>\n", cmd);
}

int main(int argc, char *argv[])
{
	char *buf;
	unsigned int i;
	int ret;
	int hint, sz_mem_to_hint;
	int sz_single_madvise, sz_single_p_madvise;
	int measure_batch;
	int err;

	if (argc != 6) {
		usage(argv[0]);
		return -1;
	}

	hint = atoi(argv[1]);
	sz_mem_to_hint = atoi(argv[2]);
	sz_single_madvise = atoi(argv[3]);
	sz_single_p_madvise = atoi(argv[4]);
	measure_batch = atoi(argv[5]);

	sz_mem_to_hint = sz_mem_to_hint / SZ_PAGE * SZ_PAGE;
	sz_single_madvise = sz_single_madvise / SZ_PAGE * SZ_PAGE;

	err = measure_madvise(hint, sz_mem_to_hint, sz_single_madvise,
			measure_batch);
	return err;
}
