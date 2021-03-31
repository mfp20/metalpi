#pragma once

#include <stdbool.h>
#include <core.h>

#if defined(__VIDEOCORE4__)

static inline void __attribute__((noreturn)) _hang_cpu() {
  __asm__ __volatile__ ("di");
  // disable every external interrupt on both  VPUs
  volatile uint32_t *maskreg = (volatile uint32_t*)(IC0_BASE + 0x10);
  for (int i=0; i<8; i++) maskreg[i] = 0;
  maskreg = (volatile uint32_t*)(IC1_BASE + 0x10);
  for (int i=0; i<8; i++) maskreg[i] = 0;
  // go to sleep forever (shouldnt need a loop, but just to be safe)
  while (true) {
    __asm__ __volatile__ ("sleep");
  }
}

#else

static inline void __attribute__((noreturn)) _hang_cpu() {
  __asm__ __volatile__ ("wfi\n");

  /* in case the above fails */
  for (;;) {
    __asm__ __volatile__ ("nop\nnop\nnop\nnop\nnop\nnop");
    __asm__ __volatile__ ("wfi\n");
  }
}

#endif

void __attribute__((noreturn)) hang_cpu(void);

