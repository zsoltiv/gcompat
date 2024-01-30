#include <fcntl.h>	/* posix_fallocate */

typedef long long off64_t;

/*
 * posix_fallocate64: gnu-ified 64-bit posix_fallocate
*/
int posix_fallocate64(int fd, off64_t offset, off64_t len)
{
	return posix_fallocate(fd, offset, len);
}
