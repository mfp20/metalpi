#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include "vc4-regs.h"
#include "miniuart.h"

bool
__vc4_data_available (void)
{
  int havedata = MMIO_READ (AUX_MU_LSR_REG);
  return havedata & 1;
}

static unsigned char
vc4_getchar (void)
{
  while (!__vc4_data_available ())
    /* empty */;

  return MMIO_READ (AUX_MU_IO_REG);
}

int
read (int fd, void *buf, unsigned int count)
{
  if (fd != 0)
    {
      errno = EBADF;
      return -1;
    }

  if (count == 0)
    return 0;

  unsigned char c = vc4_getchar ();

  if (c == '\r' && !(_miniuart_settings & MINIUART_CRLF_IN))
    c = '\n';

  if (_miniuart_settings & MINIUART_ECHO)
    __vc4_putchar (c);

  *(unsigned char *) buf = c;

  return 1;
}
