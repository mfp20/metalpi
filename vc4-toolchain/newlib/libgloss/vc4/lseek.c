#include <errno.h>

int
lseek (int fd, int offset, int whence)
{
  errno = EBADF;
  return -1;
}
