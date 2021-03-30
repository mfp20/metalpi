
#include <stdint.h>
#include <stdio.h>
#include <hardware.h>
#include <console.h>

/*----------------------------------------------*/
/* Put a character                              */
/*----------------------------------------------*/

int putchar (int c) {
  pl011_putchar(c);
}

/*----------------------------------------------*/
/* Put a null-terminated string                 */
/*----------------------------------------------*/

int __puts (				/* Put a string to the default device */
	const char* str			/* Pointer to the string */
)
{
	while (*str)
		putchar(*str++);
}

int puts (					/* Put a string to the default device */
	const char* str			/* Pointer to the string */
)
{
	__puts(str);
	putchar('\n');
}

int printf (			/* Put a formatted string to the default device */
	const char*	fmt,	/* Pointer to the format string */
	...					/* Optional arguments */
)
{
	va_list arp;

	va_start(arp, fmt);
	vprintf(fmt, arp);
	va_end(arp);
}

// Dump a line of binary dump                   */
void put_dump (
	const void* buff,		/* Pointer to the array to be dumped */
	unsigned long addr,		/* Heading address value */
	int len,				/* Number of items to be dumped */
	int width				/* Size of the items (DF_CHAR, DF_SHORT, DF_LONG) */
)
{
	int i;
	const unsigned char *bp;
	const unsigned short *sp;
	const unsigned long *lp;


	printf("%08lX ", addr);		/* address */

	switch (width) {
	case DW_CHAR:
		bp = buff;
		for (i = 0; i < len; i++)		/* Hexdecimal dump */
			printf(" %02X", bp[i]);
		putchar(' ');
		for (i = 0; i < len; i++)		/* ASCII dump */
			putchar((bp[i] >= ' ' && bp[i] <= '~') ? bp[i] : '.');
		break;
	case DW_SHORT:
		sp = buff;
		do								/* Hexdecimal dump */
			printf(" %04X", *sp++);
		while (--len);
		break;
	case DW_LONG:
		lp = buff;
		do								/* Hexdecimal dump */
			printf(" %08lX", *lp++);
		while (--len);
		break;
	}

	putchar('\n');
}

/*------------------------------------------------------------------------/
/  Universal string handler for user console interface
/-------------------------------------------------------------------------/
/
/  Copyright (C) 2011, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/
/ Formatted string output
    vprintf("%d", 1234);			"1234"
    vprintf("%6d,%3d%%", -200, 5);	"  -200,  5%"
    vprintf("%-6u", 100);			"100   "
    vprintf("%ld", 12345678L);		"12345678"
    vprintf("%04x", 0xA3);			"00a3"
    vprintf("%08LX", 0x123ABC);		"00123ABC"
    vprintf("%016b", 0x550F);		"0101010100001111"
    vprintf("%s", "String");		"String"
    vprintf("%-4s", "abc");			"abc "
    vprintf("%4s", "abc");			" abc"
    vprintf("%c", 'a');				"a"
    vprintf("%f", 10.0);            <vprintf lacks floating point support>
*/
int vprintf (
	const char*	fmt,	/* Pointer to the format string */
	va_list arp			/* Pointer to arguments */
)
{
	unsigned int r, i, j, w, f;
	unsigned long v;
	char s[16], c, d, *p;


	for (;;) {
		c = *fmt++;					/* Get a char */
		if (!c) break;				/* End of format? */
		if (c != '%') {				/* Pass through it if not a % sequense */
			putchar(c); continue;
		}
		f = 0;
		c = *fmt++;					/* Get first char of the sequense */
		if (c == '0') {				/* Flag: '0' padded */
			f = 1; c = *fmt++;
		} else {
			if (c == '-') {			/* Flag: left justified */
				f = 2; c = *fmt++;
			}
		}
		for (w = 0; c >= '0' && c <= '9'; c = *fmt++)	/* Minimum width */
			w = w * 10 + c - '0';
		if (c == 'l' || c == 'L') {	/* Prefix: Size is long int */
			f |= 4; c = *fmt++;
		}
		if (!c) break;				/* End of format? */
		d = c;
		if (d >= 'a') d -= 0x20;
		switch (d) {				/* Type is... */
		case 'S' :					/* String */
			p = va_arg(arp, char*);
			for (j = 0; p[j]; j++) ;
			while (!(f & 2) && j++ < w) putchar(' ');
			__puts(p);
			while (j++ < w) putchar(' ');
			continue;
		case 'C' :					/* Character */
			putchar((char)va_arg(arp, int)); continue;
		case 'B' :					/* Binary */
			r = 2; break;
		case 'O' :					/* Octal */
			r = 8; break;
		case 'D' :					/* Signed decimal */
		case 'U' :					/* Unsigned decimal */
			r = 10; break;
                case 'P':
                        f |= 4;
		case 'X' :					/* Hexdecimal */
			r = 16; break;
		default:					/* Unknown type (passthrough) */
			putchar(c); continue;
		}

		/* Get an argument and put it in numeral */
		v = (f & 4) ? va_arg(arp, long) : ((d == 'D') ? (long)va_arg(arp, int) : (long)va_arg(arp, unsigned int));
		if (d == 'D' && (v & 0x80000000)) {
			v = 0 - v;
			f |= 8;
		}
		i = 0;
		do {
			d = (char)(v % r); v /= r;
			if (d > 9) d += (c == 'x') ? 0x27 : 0x07;
			s[i++] = d + '0';
		} while (v && i < sizeof(s));
		if (f & 8) s[i++] = '-';
		j = i; d = (f & 1) ? '0' : ' ';
		while (!(f & 2) && j++ < w) putchar(d);
		do putchar(s[--i]); while(i);
		while (j++ < w) putchar(' ');
	}
}

void print_timestamp() {
  uint32_t clock_lo = ST_CLO;

  printf("%3ld.%06ld ", clock_lo / 1000000, clock_lo % 1000000);
}

void safe_putchar(unsigned char c) {
  if ((c >= ' ') && (c <= '~')) {
    printf("%c", c);
  } else {
    printf(".");
  }
}

// realaddr must be 16 aligned
// reads from realaddr, but claims to be from reportaddr, to allow mmap usage
// count must be a multiple of 16 bytes
void hexdump_ram(volatile void *realaddr, uint32_t reportaddr, uint32_t count) {
  volatile uint32_t *buffer_start = reinterpret_cast<volatile uint32_t*>(realaddr);
  for (uint32_t i = 0, fakeaddr = reportaddr; i < count; i += 16, fakeaddr += 16) {
    uint32_t fragment;
    printf("0x%08lx ", fakeaddr);
    for (int j=0; j<4; j++) {
      fragment = buffer_start[((i/4)+j)];
      uint8_t a,b,c,d;
      a = fragment & 0xff;
      b = (fragment >> 8) & 0xff;
      c = (fragment >> 16) & 0xff;
      d = (fragment >> 24) & 0xff;
      printf("%02x %02x %02x %02x ", a,b,c,d);
      if (j == 1) printf(" ");
    }
    printf(" |");
    for (int j=0; j<4; j++) {
      fragment = buffer_start[((i/4)+j)];
      uint8_t a,b,c,d;
      a = fragment & 0xff;
      b = (fragment >> 8) & 0xff;
      c = (fragment >> 16) & 0xff;
      d = (fragment >> 24) & 0xff;
      safe_putchar(a);
      safe_putchar(b);
      safe_putchar(c);
      safe_putchar(d);
    }
    printf("|\n");
  }
}

