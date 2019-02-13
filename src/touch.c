
#include "touch.h"
#include "vdp.h"
#include "patterns.h"
#include "tipi_mouse.h"

#define SPR_MOUSE0 0
#define SPR_MOUSE1 1

unsigned int pointerx;
unsigned int pointery;
int touch_enabled;
int left_button_pressed;
padPt touchCoord;

void touch_sprite_pos(unsigned int n, unsigned int r, unsigned int c)
{
  unsigned int addr=gSprite+(n<<2);
  VDP_SET_ADDRESS_WRITE(addr);
  VDPWD=r;
  VDPWD=c;
}

/**
 * touch_init() - Set up touch screen
 */
void touch_init(void)
{
#ifdef TIPI
  // Load Sprite patterns
  vdpmemcpy(gSpritePat, gfx_point0, 32);
  vdpmemcpy(gSpritePat + 32, gfx_point1, 32);

  pointerx = 256/2;
  pointery = 192/2;
  
  sprite(SPR_MOUSE0, 0, COLOR_MEDRED, pointery - 1, pointerx);
  sprite(SPR_MOUSE1, 4, COLOR_DKRED, pointery - 1, pointerx);
  sprite(2,0,COLOR_BLACK,208,0);
#endif
}

/* Multiply by 2.(6) = 8/3 */
unsigned int touch_scale_y(unsigned int y)
{
#ifdef TIPI
    unsigned int n, q;
    n  = y << 3;
    q  = ((y << 1) + n) >> 2;
    q += q >> 4;
    q += q >> 8;
    n -= q << 1;
    n -= q;
    n += ((n << 2) + n) << 1;
    return (q + (n >> 5) ^ 0x1FF);
#endif
}

/* Multiply by 2 = n<<4 for TI99 */
unsigned int touch_scale_x(unsigned int x)
{
  return ((x<<1));
}

/**
 * touch_main() - Main loop for touch screen
 */
void touch_main(void)
{
#ifdef TIPI
  tipiMouseRead();
  if (mousex != 0 || mousey != 0)
    {
      pointerx += (2 * mousex) / 3;
      pointery += (2 * mousey) / 3;
      
      if (pointerx > 0xF000)
	{
	  pointerx = 0;
	}
      else if (pointerx > 255)
	{
	  pointerx = 255;
	}
      
      if (pointery > 0xF000)
	{
	  pointery = 0;
	}
      else if (pointery > 191)
	{
	  pointery = 191;
	}

      if (touch_enabled==1)
	{
	  touch_sprite_pos(SPR_MOUSE0, pointery - 1, pointerx);
	  touch_sprite_pos(SPR_MOUSE1, pointery - 1, pointerx);
	}
      else
	{
	  touch_sprite_pos(SPR_MOUSE0, 208, 0);
	  touch_sprite_pos(SPR_MOUSE1, 208, 0);
	}
    }
  
  if (mouseb & MB_LEFT)
    {
      if (left_button_pressed==0)
	{
	  left_button_pressed=1;
	  touchCoord.x=touch_scale_x(pointerx);
	  touchCoord.y=touch_scale_y(pointery);
	  Touch(&touchCoord);
	}
    }
  else
    {
      left_button_pressed=0;
    }
#endif
}

/**
 * touch_allow - Set whether touchpanel is active or not.
 */
void touch_allow(padBool allow)
{
#ifdef TIPI
  touch_enabled=allow;
  if (allow==1)
    {
      touch_sprite_pos(SPR_MOUSE0, pointery - 1, pointerx);
      touch_sprite_pos(SPR_MOUSE1, pointery - 1, pointerx);
    }
  else
    {
      touch_sprite_pos(SPR_MOUSE0, 208, 0);
      touch_sprite_pos(SPR_MOUSE1, 208, 0);      
    }
#endif
}

/**
 * touch_done() - Stop the mouse driver
 */
void touch_done(void)
{
}
