
#include <stdio.h>
#include <runtime.h>
#include <core.h>

const char* size_to_string[] = {
  "1GB",
  "512MB",
  "256MB",
  "128MB",
  "2GB",
  "4GB",
  "UNKNOWN"
};

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

void __attribute__((noreturn)) hang_cpu() {
  _hang_cpu();
}

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

#endif

