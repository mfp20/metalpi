#include "sys/termios.h"
#include "miniuart.h"

extern unsigned int _miniuart_settings;

int tcgetattr(int fd, struct termios *termios_p)
{
  termios_p->c_iflag = (_miniuart_settings & MINIUART_CRLF_IN) ? INLCR : 0;
  termios_p->c_oflag = (_miniuart_settings & MINIUART_CRLF_OUT) ? ONLCR : 0;
  termios_p->c_lflag = (_miniuart_settings & MINIUART_ECHO) ? ECHO : 0;
  termios_p->c_cflag = 0;
  return 0;
}
