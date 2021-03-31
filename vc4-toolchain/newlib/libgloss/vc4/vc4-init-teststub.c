#include <stdint.h>
#include "vc4-regs.h"

/* We can't have this go in bss, else it'll be zeroed out before we hit main.
   That's what the init of 1 is for.  */
void (*__vc4_exitfn) (void) = (void (*) (void)) 1;

void
__vc4_init (void (*exitfn)(void))
{
  /* Stash this away as a way to return to the test shell.  */
  __vc4_exitfn = exitfn;

  /* We already have the mini UART set up by the boot code, no need to do it
     again.  */
}
