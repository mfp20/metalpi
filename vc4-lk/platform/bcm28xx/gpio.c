/*
 * Copyright (c) 2016 Adam Barth
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 */
#include <dev/gpio.h>
#include <errno.h>
#include <platform/bcm28xx.h>
#include <lk/reg.h>
#include <lk/console_cmd.h>
#include <lk/debug.h>
#include <string.h>
#include <platform/bcm28xx/gpio.h>
#include <platform/bcm28xx/udelay.h>

#define NUM_PINS     54
#define BITS_PER_REG 32
#define BITS_PER_PIN 3
#define PINS_PER_REG (BITS_PER_REG / BITS_PER_PIN)
#define GPIOREG(base, nr) (REG32(base) + (nr / BITS_PER_REG))

static int cmd_gpio_mode(int argc, const cmd_args *argv);
static int cmd_gpio_write(int argc, const cmd_args *argv);
static int cmd_gpio_dump_state(int argc, const cmd_args *argv);
static int cmd_gpio_test(int argc, const cmd_args *argv);

STATIC_COMMAND_START
STATIC_COMMAND("gpio_mode", "set the gpio alt mode", &cmd_gpio_mode)
STATIC_COMMAND("gpio_write", "set gpio output", &cmd_gpio_write)
STATIC_COMMAND("gpio_dump_state", "dump current gpio state", &cmd_gpio_dump_state)
STATIC_COMMAND("gpio_test", "test gpio pullup/down", &cmd_gpio_test)
STATIC_COMMAND_END(gpio);

int gpio_config(unsigned nr, unsigned flags) {
    unsigned mask = 0x7;
    if (nr >= NUM_PINS || flags & ~mask)
        return -EINVAL;
    unsigned register_number = nr / PINS_PER_REG;
    unsigned offset = (nr % PINS_PER_REG) * BITS_PER_PIN;
    unsigned shifted_mask = mask << offset;
    volatile uint32_t *reg = REG32(GPIO_GPFSEL0) + register_number;
    *reg = (*reg & ~shifted_mask) | (flags << offset);
    return 0;
}

int gpio_get_config(unsigned nr) {
    unsigned mask = 0x7;
    if (nr >= NUM_PINS)
        return -EINVAL;
    unsigned register_number = nr / PINS_PER_REG;
    unsigned offset = (nr % PINS_PER_REG) * BITS_PER_PIN;
    unsigned shifted_mask = mask << offset;
    volatile uint32_t *reg = REG32(GPIO_GPFSEL0) + register_number;
    return (*reg & shifted_mask) >> offset;
}

void gpio_set(unsigned nr, unsigned on) {
    unsigned offset = nr % BITS_PER_REG;
    *GPIOREG(on ? GPIO_GPSET0 : GPIO_GPCLR0, nr) = 1 << offset;
}

int gpio_get(unsigned nr) {
    unsigned offset = nr % BITS_PER_REG;
    return (*GPIOREG(GPIO_GPLEV0, nr) & (1 << offset)) >> offset;
}

#ifndef RPI4
// rpi1-3 models
void gpio_set_pull(unsigned nr, enum pull_mode mode) {
}
void gpio_apply_batch(struct gpio_pull_batch *batch) {
  for (enum pull_mode mode = 0; mode <=2; mode++) {
    if (batch->bank[mode][0] | batch->bank[mode][1]) {
      *REG32(GPIO_GPPUD) = mode;
      udelay(500);
      *REG32(GPIO_GPPUDCLK0) = batch->bank[mode][0];
      *REG32(GPIO_GPPUDCLK1) = batch->bank[mode][1];
      udelay(500);
      *REG32(GPIO_GPPUDCLK0) = 0;
      *REG32(GPIO_GPPUDCLK1) = 0;
      *REG32(GPIO_GPPUD) = 0;
      udelay(500);
    }
  }
}
#else
// rpi4
int gpio_get_pull(unsigned nr) {
  unsigned offset = (nr % 16) * 2;
  volatile uint32_t *pull_cfg = REG32(GPIO_2711_PULL);
  return (pull_cfg[nr / 16] >> offset) & 0x3;
}
#endif

static int cmd_gpio_mode(int argc, const cmd_args *argv) {
  if (argc != 3) {
    printf("usage: gpio_mode 42 (in|out|alt0|alt1|alt2|alt3|alt4|alt5)\n");
    return -1;
  }
  int mode = 0;
  if (strcmp(argv[2].str, "in") == 0) {
    mode = 0;
  } else if (strcmp(argv[2].str, "out") == 0) {
    mode = 1;
  } else if (strcmp(argv[2].str, "alt0") == 0) {
    mode = 4;
  } else if (strcmp(argv[2].str, "alt1") == 0) {
    mode = 5;
  } else if (strcmp(argv[2].str, "alt2") == 0) {
    mode = 6;
  } else if (strcmp(argv[2].str, "alt3") == 0) {
    mode = 7;
  } else if (strcmp(argv[2].str, "alt4") == 0) {
    mode = 3;
  } else if (strcmp(argv[2].str, "alt5") == 0) {
    mode = 2;
  } else {
    printf("unknown mode %s\n", argv[2].str);
    return -1;
  }
  gpio_config(argv[1].u, mode);
  printf("done\n");
  return 0;
}

static int cmd_gpio_write(int argc, const cmd_args *argv) {
  if (argc != 3) {
    printf("usage: gpio_write 42 1\n");
    return -1;
  }
  gpio_set(argv[1].u, argv[2].u);
  printf("done\n");
  return 0;
}


const char *mode_names[] = {
  [kBCM2708PinmuxIn] = "IN",
  [kBCM2708PinmuxOut] = "OUT",
  [kBCM2708Pinmux_ALT5] = "ALT5",
  [kBCM2708Pinmux_ALT4] = "ALT4",
  [kBCM2708Pinmux_ALT0] = "ALT0",
  [kBCM2708Pinmux_ALT1] = "ALT1",
  [kBCM2708Pinmux_ALT2] = "ALT2",
  [kBCM2708Pinmux_ALT3] = "ALT3",
  [8] = ""
};

const char *levels[] = { "LOW", "HIGH" };

const char *pull_names[] = { "--", "^^", "\\/", "invalid", "" };

static int cmd_gpio_dump_state(int argc, const cmd_args *argv) {
  for (int i=0; i<32; i++) {
    int lflags, rflags = 8;
    lflags = gpio_get_config(i);
    int l_level = gpio_get(i);
    int r_level = gpio_get(i+32);
    if ((i+32) < NUM_PINS) rflags = gpio_get_config(i+32);

#if RPI4
    int l_pull = gpio_get_pull(i);
    int r_pull = gpio_get_pull(i+32);
#else
    int l_pull = 4, r_pull = 4;
#endif

    printf("GPIO%02d %4s %s %4s | %4s %s %4s GPIO%02d\n", i, mode_names[lflags], pull_names[l_pull], levels[l_level],
        levels[r_level], pull_names[r_pull], mode_names[rflags], i+32);
  }
  return 0;
}

static int cmd_gpio_test(int argc, const cmd_args *argv) {
  uint8_t pins[] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27 };
  for (unsigned int i=0; i<sizeof(pins); i++) {
    int mode = gpio_get_config(pins[i]);
    printf("GPIO%02d Mode:%s ", pins[i], mode_names[mode]);
    struct gpio_pull_batch pullBatch;
    GPIO_PULL_CLEAR(pullBatch);
    GPIO_PULL_SET(pullBatch, pins[i], kPullDown);
    gpio_apply_batch(&pullBatch);
    printf("DOWN %d %08x ", gpio_get(pins[i]), *REG32(GPIO_GPLEV0));

    GPIO_PULL_CLEAR(pullBatch);
    GPIO_PULL_SET(pullBatch, pins[i], kPullUp);
    gpio_apply_batch(&pullBatch);
    printf("UP %d %08x\n", gpio_get(pins[i]), *REG32(GPIO_GPLEV0));
  }
  return 0;
}
