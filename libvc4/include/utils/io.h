
#include <stdint.h>

void print_timestamp(void);
void put_dump (const void* buff, unsigned long addr, int len, int width);
void safe_putchar(unsigned char c);
void hexdump_ram(volatile void *realaddr, uint32_t reportaddr, uint32_t count);

