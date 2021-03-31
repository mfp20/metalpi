#include <errno.h>
#include <stdint.h>
#include "vc4-regs.h"
#include "miniuart.h"

unsigned int _miniuart_settings;

void
__vc4_putchar (unsigned int c)
{
  unsigned int tmp;
  
  do
    {
      tmp = MMIO_READ (AUX_MU_LSR_REG);
    }
  while ((tmp & 0x20) != 0x20);
  
  MMIO_WRITE (AUX_MU_IO_REG, c);
}

int
write (int fd, const void *buf, unsigned int count)
{
  int i;
  const char *bufc = buf;

  if (fd != 1 && fd != 2)
    {
      errno = EBADF;
      return -1;
    }

  for (i = 0; i < count; i++)
    {
      char c = bufc[i];
      __vc4_putchar (c);
      if (c == '\n' && (_miniuart_settings & MINIUART_CRLF_OUT))
        __vc4_putchar ('\r');
    }

  return count;
}
