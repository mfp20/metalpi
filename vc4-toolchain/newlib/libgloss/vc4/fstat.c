#include <errno.h>

int
fstat (int fd, void *buf)
{
  errno = EBADF;
  return -1;
}
