__attribute__((noreturn)) void
_exit (int status)
{
  register int r0 __asm__("r0") = status;
  __asm__ __volatile__ ("sleep" : : "r" (r0));
}
