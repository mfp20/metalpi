#ifndef MINIUART_H
#define MINIUART_H 1

#include <stdbool.h>

#define MINIUART_CRLF_IN  1
#define MINIUART_CRLF_OUT 2
#define MINIUART_ECHO     4

extern unsigned int _miniuart_settings;

extern bool __vc4_data_available (void);
extern void __vc4_putchar (unsigned int c);

#endif
