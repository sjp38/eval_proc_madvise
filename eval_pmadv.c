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

int measure_p_madvise(int hint, int sz_mem_to_hint,
		int sz_single_madvise, int sz_single_p_madvise)
{
	pid_t pid = getpid();
	int pidfd = syscall(SYS_pidfd_open, pid, 0);
	struct iovec *vec;
	char *buf;
	int i, err, start;
	struct timespec measure_start, measure_end;
	int len_vec = sz_single_p_madvise / sz_single_madvise;
	int ret;
	unsigned long elapsed_ns = 0;
	unsigned long nr_measures = 0;

	if (pidfd == -1) {
		perror("pidfd_open fail");
		return -1;
	}

	buf = mmap(NULL, sz_mem_to_hint, PROT_READ | PROT_WRITE, MAP_PRIVATE |
			MAP_ANON, -1, 0);
	if (buf == MAP_FAILED) {
		perror("mmap fail");
		goto out;
	}

	vec = malloc(sizeof(*vec) * len_vec);
	if (!vec) {
		perror("iovec alloc fail");
		goto free_buf_out;
	}

	while (elapsed_ns < 5ul * 1000 * 1000 * 1000) {
		memset(buf, 1, sz_mem_to_hint);

		err = clock_gettime(CLOCK_MONOTONIC, &measure_start);
		if (err) {
			perror("clock_gettime() failed\n");
			goto free_vec_out;
		}
		for (start = 0; start < sz_mem_to_hint;
				start += sz_single_p_madvise) {
			int j;

			for (j = 0; j < len_vec; j++) {
				vec[j].iov_base = &buf[
					start + j * sz_single_madvise];
				vec[j].iov_len = sz_single_madvise;
			}
			ret = syscall(SYS_process_madvise, pidfd, vec, len_vec, hint, 0);
			if (ret != len_vec * sz_single_madvise) {
				perror("process_madivse fail\n");
				goto free_vec_out;
			}
		}
		err = clock_gettime(CLOCK_MONOTONIC, &measure_end);
		if (err) {
			perror("clock_gettime() failed\n");
			goto free_vec_out;
		}
		elapsed_ns += measure_end.tv_sec * 1000000000 + measure_end.tv_nsec
			- measure_start.tv_sec * 1000000000 -
			measure_start.tv_nsec;
		nr_measures++;

	}
	printf("%lu\n", elapsed_ns / nr_measures);

free_vec_out:
	free(vec);
free_buf_out:
	err = munmap(buf, sz_mem_to_hint);
	if (err)
		perror("munamap fail");
out:
	close(pidfd);
	return err;
}

void usage(char *cmd)
{
	printf("usage: %s <hint> <sz_mem> <sz_madv> <sz_p_madv>\n", cmd);
}

int main(int argc, char *argv[])
{
	char *buf;
	unsigned int i;
	int ret;
	int hint, sz_mem_to_hint;
	int sz_single_madvise, sz_single_p_madvise;
	int err;

	if (argc != 5) {
		usage(argv[0]);
		return -1;
	}

	hint = atoi(argv[1]);
	sz_mem_to_hint = atoi(argv[2]);
	sz_single_madvise = atoi(argv[3]);
	sz_single_p_madvise = atoi(argv[4]);

	sz_mem_to_hint = sz_mem_to_hint / SZ_PAGE * SZ_PAGE;
	sz_single_madvise = sz_single_madvise / SZ_PAGE * SZ_PAGE;
	err = measure_p_madvise(hint, sz_mem_to_hint, sz_single_madvise,
			sz_single_p_madvise);
	return err;
}
