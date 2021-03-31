#include <core/cpu.h>

void __attribute__((noreturn)) hang_cpu() {
  _hang_cpu();
}

