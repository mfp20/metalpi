#include <stdio.h>

int main(void)
{
  int status;
  __asm__ __volatile__ ("mov %0,sr" : "=r" (status));
  printf("status: %x\n", status);
  __asm__ __volatile__ ("bkpt\n\tnop\n\tnop");
//  __asm__ __volatile__ ("mov sr, #0x60000008");
  __asm__ __volatile__ ("mov %0,sr" : "=r" (status));
  printf("status: %x\n", status);
  return 0;
}
