// Universal string handler for user console interface  (C)ChaN, 2011
//
// use putchar_ptr to hook the console to an hardware device
// example for pl011 uart having void pl011_putchar(unsigned char c):
//		putchar_ptr = pl011_putchar;

#pragma once

#include <stdint.h>

#ifdef __arm__
	#include <stdarg.h>
#else
	#include "stdarg.h"
#endif

#ifndef _STRFUNC
#define _STRFUNC

#define _USE_XFUNC_OUT	1	/* 1: Use output functions */
#define	_CR_CRLF		1	/* 1: Convert \n ==> \r\n in the output char */
#define DW_CHAR		sizeof(char)
#define DW_SHORT	sizeof(short)
#define DW_LONG		sizeof(long)

void (*putchar_ptr)(unsigned char c);

int putchar (int c);
int __puts(const char *s);
int puts (const char* str);
int vprintf (const char *fmt, va_list arp);
int printf (const char* fmt, ...) __attribute__ ((format (printf, 1, 2)));
void put_dump (const void* buff, unsigned long addr, int len, int width);

#endif

void print_timestamp(void);
void safe_putchar(unsigned char c);
void hexdump_ram(volatile void *realaddr, uint32_t reportaddr, uint32_t count);
