/*=============================================================================
Copyright (C) 2016-2017 Authors of rpi-open-firmware
Copyright (C) 2021 Anichang
All rights reserved.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

FILE DESCRIPTION
- Glue code for Broadcom's register definitions as well as certain registers
that are missing from the release. This is also used by ARM.
- Simple timer based delay routine.
- Halt route
- Panic routine.

=============================================================================*/
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <core/broadcom.h>

#ifdef __arm__
# ifdef BAREMETAL
#   define HAS_DYNAMIC_ALLOCATIONS
# endif
#endif

#ifdef HAS_DYNAMIC_ALLOCATIONS
#include <tlsf/tlsf.h>
#endif

#ifdef __VIDEOCORE4__
#include <vc4_types.h>
#endif

#define RegToRef(x) reinterpret_cast<volatile uint32_t*>(&x)

#define VC4_PERIPH_BASE 0x7E000000
#define ARM_PERIPH_BASE 0x3F000000
#define VC4_TO_ARM_PERIPH(addr) ((addr - VC4_PERIPH_BASE) + ARM_PERIPH_BASE)
#define VC4_CPUID_BCM2709_PLUS 0x40

#if defined(__arm__) | defined(__aarch64__)
#  ifdef BAREMETAL
#    define HW_REGISTER_RW(addr) (*(volatile uint32_t *)(VC4_TO_ARM_PERIPH(addr)))
#    define HW_REGISTER_RO(addr) (*(const volatile uint32_t *)(VC4_TO_ARM_PERIPH(addr)))
#  else
#    define HW_REGISTER_RW(addr) (*(volatile uint32_t *)(mmiobase + (addr & 0x00ffffff)))
#  endif
#else
#define HW_REGISTER_RW(addr) (*(volatile uint32_t *)(addr))
#define HW_REGISTER_RO(addr) (*(const volatile uint32_t *)(addr))
#endif

#define mmio_read32(addr) HW_REGISTER_RW(addr)
#define mmio_write32(addr, value) (HW_REGISTER_RW(addr) = value)

// LPDDR mode registers.
#define LPDDR2_MR_DEVICE_INFO      0
#define LPDDR2_MR_DEVICE_FEATURE_1 1
#define LPDDR2_MR_DEVICE_FEATURE_2 2
#define LPDDR2_MR_IO_CONFIG        3
#define LPDDR2_MR_TEMPERATURE      4
#define LPDDR2_MR_MANUFACTURER_ID  5
#define LPDDR2_MR_REV_1            6
#define LPDDR2_MR_REV_2            7
#define LPDDR2_MR_METRICS          8
#define LPDDR2_MR_CALIBRATION      10

#define CM_SRC_GND			0
#define CM_SRC_OSC			1
#define CM_SRC_TESTDEBUG0		2
#define CM_SRC_TESTDEBUG1		3
#define CM_SRC_PLLA_CORE		4
#define CM_SRC_PLLA_PER		4
#define CM_SRC_PLLC_CORE0		5
#define CM_SRC_PLLC_PER		5
#define CM_SRC_PLLC_CORE1		8
#define CM_SRC_PLLD_CORE		6
#define CM_SRC_PLLD_PER		6
#define CM_SRC_PLLH_AUX		7
#define CM_SRC_PLLC_CORE1		8
#define CM_SRC_PLLC_CORE2		9

#define panic_plain(ex, ...) \
	(panic)(ex, ## __VA_ARGS__)
#define __STRINGIFY(x) #x
#define LINE_NUMBER(x) __STRINGIFY(x)
#define PANIC_LOCATION __FILE__ ":" LINE_NUMBER(__LINE__)
#define panic(ex, ...) \
	(panic)(# ex "@" PANIC_LOCATION, ## __VA_ARGS__)

#define assert(x) if (!(x)) { panic("assertion '%s' failed", #x); }

enum RamSize {
  kRamSize1GB = 0,
  kRamSize512MB = 1,
  kRamSize256MB = 2,
  kRamSize128MB = 3,
  kRamSize2GB = 4,
  kRamSize4GB = 5,
  kRamSizeUnknown
};

typedef struct {
  uint32_t sdram_size;
  uint32_t vpu_cpuid;
  uint32_t serial;
  uint32_t revision;
  uint32_t reserved[1];
} firmware_arm_data_t;
#ifdef __arm__
extern firmware_arm_data_t g_FirmwareData;
#endif

extern const char *size_to_string[];

void udelay(uint32_t time);

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
void __attribute__((noreturn)) __attribute__ ((format (printf, 1, 2))) panic(const char* fmt,  ...);

