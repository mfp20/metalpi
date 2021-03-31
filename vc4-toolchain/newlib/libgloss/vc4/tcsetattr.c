#include "sys/termios.h"
#include "miniuart.h"

extern unsigned int _miniuart_settings;

int tcsetattr(int fd, int action, const struct termios *termios_p)
{
  _miniuart_settings = 0;

  if (termios_p->c_iflag & INLCR)
    _miniuart_settings |= MINIUART_CRLF_IN;

  if ((termios_p->c_oflag & (OPOST | ONLCR)) == OPOST | ONLCR)
    _miniuart_settings |= MINIUART_CRLF_OUT;

  if (termios_p->c_lflag & ECHO)
    _miniuart_settings |= MINIUART_ECHO;

  return 0;
}
