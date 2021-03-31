#include <errno.h>

int
close (int fd)
{
  errno = EBADF;
  return -1;
}
