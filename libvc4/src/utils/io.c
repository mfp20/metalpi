
#include <stdint.h>
#include <stdio.h>
#include <hardware.h>
#include <io.h>

void print_timestamp() {
  uint32_t clock_lo = ST_CLO;

  printf("%3ld.%06ld ", clock_lo / 1000000, clock_lo % 1000000);
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

