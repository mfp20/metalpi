#include <sys/time.h>
#include <stdbool.h>
#include <stdint.h>
#include "vc4-regs.h"
#include "miniuart.h"

int
select (int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
        struct timeval *timeout)
{
  bool waitforinput = false;

  if (FD_ISSET (0, readfds))
    waitforinput = true;

  FD_ZERO (readfds);
  FD_ZERO (writefds);
  FD_ZERO (exceptfds);

  if (timeout)
    {
      uint32_t ticks = timeout->tv_sec * 10000000 + timeout->tv_usec;
      uint32_t start = MMIO_READ (ST_CLO);
      while (MMIO_READ (ST_CLO) - start < ticks)
        /* empty */;
    }
  else
    {
      if (waitforinput)
        while (!__vc4_data_available ())
          /* empty */;
    }

  if (waitforinput && __vc4_data_available ())
    {
      FD_SET (0, readfds);
      return 1;
    }

  return 0;
}
