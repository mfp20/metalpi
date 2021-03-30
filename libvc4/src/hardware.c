
#include <stdio.h>
#include <runtime.h>
#include <hardware.h>
#include <hardware/pll_read.h>
#if defined(__VIDEOCORE4__)
	#include <cpu.h>
#else
	#include <hardware/hang_cpu.h>
#endif

#define UART_IBRD *((volatile uint32_t*)(0x7e201024))
#define UART_FBRD *((volatile uint32_t*)(0x7e201028))
#define UART_LCRH *((volatile uint32_t*)(0x7e20102c))
#define UART_CR   *((volatile uint32_t*)(0x7e201030))
#define UART_ICR  *((volatile uint32_t*)(0x7e201044))

const char* size_to_string[] = {
  "1GB",
  "512MB",
  "256MB",
  "128MB",
  "2GB",
  "4GB",
  "UNKNOWN"
};

static char buffer[512];

#undef panic
void panic(const char* fmt,  ...) {
	printf("panic(): ");

	va_list arp;

	va_start(arp, fmt);
	vprintf(fmt, arp);
	va_end(arp);

	putchar('\n');

	hang_cpu();
}

// not sure what controls the rate but i have confirmed that ST_CLO counts micro-seconds
//		ie: calling udelay(10 * 1000 * 1000) results in a stall of ~10 seconds
// this is true for both vpu and arm
void udelay(uint32_t t) {
	uint32_t tv = ST_CLO;
	for (;;) {
		/* nop still takes a cycle i think? */
		__asm__ __volatile__ ("nop" :::);
		if ((ST_CLO - tv) > t)
			return;
	}
}

#ifdef BAREMETAL
//FILE *funopen(void *cookie, int (*read)(void*, char *, int), int (*write)(void*, const char*, int), int, int);

static void pl011_flush() {
  while(UART_MSR & 0x20);
}

void pl011_putchar(unsigned char c) {
  if (c == '\n') pl011_putchar('\r');
  pl011_flush();
  UART_RBRTHRDLL = c;
}

static int uart_write(void *uart, const char *str, int length) {
  for (int i=0; i<length; i++) {
    pl011_putchar(str[i]);
  }
  return length;
}

void pl011_uart_init(uint32_t baud) {
  pl011_flush();
  UART_CR = 0;

  if (CM_UARTDIV == 0) {
    // CM_UARTDIV can range from 0 to 1023 with a fractional resolution of 1/4096th
    // on the rpi1-3, this sets the freq to 19.2 / (0x6666 / 0x1000) == ~3mhz
    // TODO, have a better default for other models?
    CM_UARTDIV = CM_PASSWORD | 0x3900;
    CM_UARTCTL = CM_PASSWORD | CM_SRC_OSC | CM_UARTCTL_FRAC_SET | CM_UARTCTL_ENAB_SET;
  }
  uint32_t uart_freq = get_uart_base_freq();
  uint32_t divisor = (uart_freq << 6) / baud / 16;

  UART_ICR = 0x7ff; // clear all interrupt flags
  UART_LCRH = 0x70; // fifo enable, 8bit mode

  // 0-65525 range, with 1/64th resolution
  UART_IBRD = (divisor >> 6) & 0xffff;
  UART_FBRD = divisor & 0x3f;
  UART_CR = 0x301;

  stdout = funopen(0, 0, uart_write, 0, 0);
  setvbuf(stdout, buffer, _IOLBF, 512);
  //printf("baud set to %ld with a divisor of %ld\n", (uart_freq << 6) / divisor / 16, divisor);
}

#endif
