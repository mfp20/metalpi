#include <stdint.h>
#include "vc4-regs.h"

static void
delay (int delayfor)
{
  int i;
  for (i = 0; i < delayfor; i++)
    __asm__ __volatile__ ("nop");
}

void
__vc4_init (void)
{
  uintptr_t tmp;

  /* Configure RX and TX GPIO pins for mini-UART function.  */
  tmp = MMIO_READ (GPFSEL1);
  tmp &= ~(7 << 12);
  tmp |= 2 << 12;
  tmp &= ~(7 << 15);
  tmp |= 2 << 15;
  MMIO_WRITE (GPFSEL1, tmp);

  MMIO_WRITE (GPPUD, 0);

  delay (150);

  MMIO_WRITE (GPPUDCLK0, (1 << 14) | (1 << 15));

  delay (150);

  MMIO_WRITE (AUX_ENABLES, 1);
  MMIO_WRITE (AUX_MU_IER_REG, 0);
  MMIO_WRITE (AUX_MU_CNTL_REG, 0);
  MMIO_WRITE (AUX_MU_LCR_REG, 3);
  MMIO_WRITE (AUX_MU_MCR_REG, 0);
  MMIO_WRITE (AUX_MU_IER_REG, 0);
  MMIO_WRITE (AUX_MU_IIR_REG, 0x6c);
  MMIO_WRITE (AUX_MU_BAUD_REG, SYSTEM_CLOCK / (TARGET_BAUD_RATE * 8) - 1);
  MMIO_WRITE (AUX_MU_LCR_REG, 3);
  MMIO_WRITE (AUX_MU_CNTL_REG, 3);
}
