#include <errno.h>

int
isatty (int fd)
{
  if (fd == 1 || fd == 2)
    return 1;

  errno = EBADF;

  return 0;
}
