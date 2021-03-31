extern void (*__vc4_exitfn) (void);

__attribute__((noreturn)) void
_exit (int status __attribute__((ignore)))
{
  __vc4_exitfn ();
}
