#ifndef VC4_REGS_H
#define VC4_REGS_H 1

#define GPFSEL1		0x200004
#define GPSET0		0x20001C
#define GPCLR0		0x200028
#define GPPUD		0x200094
#define GPPUDCLK0	0x200098

#define AUX_ENABLES	0x215004
#define AUX_MU_IO_REG	0x215040
#define AUX_MU_IER_REG	0x215044
#define AUX_MU_IIR_REG	0x215048
#define AUX_MU_LCR_REG	0x21504C
#define AUX_MU_MCR_REG	0x215050
#define AUX_MU_LSR_REG	0x215054
#define AUX_MU_MSR_REG	0x215058
#define AUX_MU_SCRATCH	0x21505C
#define AUX_MU_CNTL_REG	0x215060
#define AUX_MU_STAT_REG	0x215064
#define AUX_MU_BAUD_REG	0x215068

#define MMIO_BASE	0x7e000000

#define MMIO_READ(X) (*(volatile uintptr_t *) (MMIO_BASE + (X)))
#define MMIO_WRITE(X,V) (*(volatile uintptr_t *) (MMIO_BASE + (X))) = (V)

//#define SYSTEM_CLOCK		19200000
#define SYSTEM_CLOCK		250000000
#define TARGET_BAUD_RATE	115200

#define ST_CLO                  0x3004

#endif
