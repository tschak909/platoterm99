#include <stdbool.h>
#include "system.h"
#include "protocol.h"
#include "conio.h"
#include "screen.h"
#include "io.h"
#include "terminal.h"
#include "keyboard.h"
#include "splash.h"
#include "vdp.h"
#include "touch.h"

unsigned char already_started=false;

void main(void)
{
  screen_init();
  touch_init();
  ShowPLATO(splash,sizeof(splash));
  terminal_initial_position();
  io_init();
  for (;;)
    {
      VDP_INT_POLL;
      VDP_WAIT_VBLANK_CRU;
      keyboard_main();
      io_main();
      touch_main();
    }
}
