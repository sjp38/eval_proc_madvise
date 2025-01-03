#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/uio.h>
#include <unistd.h>

#define SZ_PAGE (4096)
#define NR_PAGES (16)
#define MMAP_SZ	(SZ_PAGE * NR_PAGES)

int main(void)
{
	char *buf;
	unsigned int i;
	int ret;
	pid_t pid = getpid();
	int pidfd = syscall(SYS_pidfd_open, pid, 0);
	struct iovec *vec;
	buf = mmap(NULL, MMAP_SZ, PROT_READ | PROT_WRITE, MAP_PRIVATE |
			MAP_ANON, -1, 0);
	if (buf == MAP_FAILED) {
		printf("mmap fail\n");
		return -1;
	}
	vec = malloc(sizeof(*vec) * NR_PAGES);
	for (i = 0; i < NR_PAGES; i++) {
		vec[i].iov_base = &buf[i * SZ_PAGE];
		vec[i].iov_len = SZ_PAGE;
	}
	ret = syscall(SYS_process_madvise, pidfd, vec, NR_PAGES,
			MADV_DONTNEED, 0);
	if (ret != MMAP_SZ) {
		printf("process_madvise fail\n");
		return -1;
	}
	ret = munmap(buf, MMAP_SZ);
	if (ret) {
		printf("munmap failed\n");
		return -1;
	}
	close(pidfd);
	printf("good\n");
	return 0;
}
