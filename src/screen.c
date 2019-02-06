#include "font.h"
#include "protocol.h"
#include "conio.h"
#include "math.h"
#include "io.h"
#include "sound.h"

unsigned char CharWide=8;
unsigned char CharHigh=16;
padPt TTYLoc;
padPt statusLoc={0,0};
extern padBool FastText; /* protocol.c */
extern unsigned char font[];
extern unsigned char fontm23[];
extern unsigned char FONT_SIZE_X;
extern unsigned char FONT_SIZE_Y;
extern BaudRate io_baud_rate;

// Miscellaneous math functions needed for coordinate translation.
short max(short a, short b) { return ( a > b ) ? a : b; }
short min(short a, short b) { return ( a < b ) ? a : b; }
short scaley(short y) { return (((y^0x01FF) << 1) * 3) >> 4; }
short scalex(short x) { return (x >> 1); }

#define FONTPTR(a) (((a << 1) + a) << 1)

/**
 * sndplay(sndlist, len) - Copy sound list into VDP and play
 */
void sndplay(unsigned char* sndlst, int len)
{        
    int vaddr = 0x1000; // address of a vdp buffer you know of, to put the sound list into

    vdpmemcpy(vaddr, sndlst, len);
    SET_SOUND_PTR(vaddr);
    SET_SOUND_VDP();
    START_SOUND();

    // run the interrupt and wait for the sound to stop
    while (SOUND_CNT) {
        VDP_INT_POLL;
    }
}

/**
 * screen_init() - Set up the screen
 */
void screen_init(void)
{
  bm_consolefont();
  set_bitmap(0);
  bm_setbackground(COLOR_CYAN);
  bm_setforeground(COLOR_BLACK);
  bordercolor(COLOR_CYAN);
  bgcolor(COLOR_CYAN);
  bm_clearscreen();
}

/**
 * screen_cycle_foreground()
 * Go to the next foreground color in palette
 */
void screen_cycle_foreground(void)
{
}

/**
 * screen_cycle_background()
 * Go to the next background color in palette
 */
void screen_cycle_background(void)
{
}

/**
 * screen_cycle_border()
 * Go to the next border color in palette
 */
void screen_cycle_border(void)
{
}

/**
 * screen_update_colors() - Set the terminal colors
 */
void screen_update_colors(void)
{
}

/**
 * screen_wait(void) - Sleep for approx 16.67ms
 */
void screen_wait(void)
{
}

/**
 * screen_beep(void) - Beep the terminal
 */
void screen_beep(void)
{
    char beep[] = { 3, 0x80, 0x05, 0x91, 10, 
    	1, 0x9f, 0 
    };
  sndplay(&beep[0], sizeof(beep));
}

/**
 * screen_clear - Clear the screen
 */
void screen_clear(void)
{
  bm_clearscreen();
}

/**
 * screen_set_pen_mode(void) - Set the current pen mode
 */
void screen_set_pen_mode(void)
{
  if (CurMode==ModeErase || CurMode==ModeInverse)
    {
      bm_setforeground(COLOR_CYAN);
    }
  else
    {
      bm_setforeground(COLOR_BLACK);
    }
}

/**
 * screen_block_draw(Coord1, Coord2) - Perform a block fill from Coord1 to Coord2
 */
void screen_block_draw(padPt* Coord1, padPt* Coord2)
{
  int y;
  int mode;
  int x1=min(scalex(Coord1->x),scalex(Coord2->x));
  int x2=max(scalex(Coord1->x),scalex(Coord2->x));
  int y1=min(scaley(Coord1->y),scaley(Coord2->y));
  int y2=max(scaley(Coord1->y),scaley(Coord2->y));

  screen_set_pen_mode();

  if (CurMode==ModeErase)
    mode=0;
  else
    mode=1;

  for (y=y1;y<y2;y++)
    {
      bm_drawline(x1,y,x2,y,0);
    }
}

/**
 * screen_dot_draw(Coord) - Plot a mode 0 pixel
 */
void screen_dot_draw(padPt* Coord)
{
  screen_set_pen_mode();
  if (CurMode==ModeErase)
    bm_clearpixel(scalex(Coord->x),scaley(Coord->y));
  else
    bm_setpixel(scalex(Coord->x),scaley(Coord->y));
}

/**
 * screen_line_draw(Coord1, Coord2) - Draw a mode 1 line
 */
void screen_line_draw(padPt* Coord1, padPt* Coord2)
{
  unsigned short x1=scalex(Coord1->x);
  unsigned short x2=scalex(Coord2->x);
  unsigned short y1=scaley(Coord1->y);
  unsigned short y2=scaley(Coord2->y);
  
  screen_set_pen_mode();
  if (CurMode==ModeErase)
    bm_drawline(x1,y1,x2,y2,0);
  else
    bm_drawline(x1,y1,x2,y2,1);

}

/**
 * screen_char_draw(Coord, ch, count) - Output buffer from ch* of length count as PLATO characters
 */
void screen_char_draw(padPt* Coord, unsigned char* ch, unsigned char count)
{
  short offset; /* due to negative offsets */
  unsigned short x;      /* Current X and Y coordinates */
  unsigned short y;
  unsigned short* px;   /* Pointers to X and Y coordinates used for actual plotting */
  unsigned short* py;
  unsigned char i; /* current character counter */
  unsigned char a; /* current character byte */
  unsigned char j,k; /* loop counters */
  char b; /* current character row bit signed */
  unsigned char width=FONT_SIZE_X;
  unsigned char height=FONT_SIZE_Y;
  unsigned short deltaX=1;
  unsigned short deltaY=1;
  unsigned char mainColor=COLOR_BLACK;
  unsigned char altColor=COLOR_CYAN;
  unsigned char *p;
  unsigned char* curfont;
  
  switch(CurMem)
    {
    case M0:
      curfont=font;
      offset=-32;
      break;
    case M1:
      curfont=font;
      offset=64;
      break;
    case M2:
      curfont=fontm23;
      offset=-32;
      break;
    case M3:
      curfont=fontm23;
      offset=32;      
      break;
    }

  if (CurMode==ModeRewrite)
    {
      altColor=COLOR_CYAN;
    }
  else if (CurMode==ModeInverse)
    {
      altColor=COLOR_BLACK;
    }
  
  if (CurMode==ModeErase || CurMode==ModeInverse)
    mainColor=COLOR_CYAN;
  else
    mainColor=COLOR_BLACK;

  bm_setforeground(mainColor);
  bm_setbackground(altColor);
  
  x=scalex((Coord->x&0x1FF));
  if (ModeBold)
    y=scaley((Coord->y+30)&0x1FF);
  else
    y=scaley((Coord->y+15)&0x1FF);

  if (FastText==padF)
    {
      goto chardraw_with_fries;
    }

  /* the diet chardraw routine - fast text output. */
  
  for (i=0;i<count;++i)
    {
      a=*ch;
      ++ch;
      a+=offset;
      p=&curfont[FONTPTR(a)];
      
      for (j=0;j<FONT_SIZE_Y;++j)
  	{
  	  b=*p;
	  
  	  for (k=0;k<FONT_SIZE_X;++k)
  	    {
  	      if (b<0) /* check sign bit. */
		{
		  bm_setforeground(mainColor);
		  if (CurMode==ModeErase)
		    bm_clearpixel(x,y);
		  else
		    bm_setpixel(x,y);
		}

	      ++x;
  	      b<<=1;
  	    }

	  ++y;
	  x-=width;
	  ++p;
  	}

      x+=width;
      y-=height;
    }

  return;

 chardraw_with_fries:
  if (Rotate)
    {
      deltaX=-abs(deltaX);
      width=-abs(width);
      px=&y;
      py=&x;
    }
    else
    {
      px=&x;
      py=&y;
    }
  
  if (ModeBold)
    {
      deltaX = deltaY = 2;
      width<<=1;
      height<<=1;
    }
  
  for (i=0;i<count;++i)
    {
      a=*ch;
      ++ch;
      a+=offset;
      p=&curfont[FONTPTR(a)];
      for (j=0;j<FONT_SIZE_Y;++j)
  	{
  	  b=*p;

	  if (Rotate)
	    {
	      px=&y;
	      py=&x;
	    }
	  else
	    {
	      px=&x;
	      py=&y;
	    }

	  // special 9918 specific hack for inverse video
	  if (CurMode==ModeInverse)
	    {
	      for (k=0;k<FONT_SIZE_X;++k)
		{
		  if (ModeBold)
		    {
		      bm_setpixel(*px+1,*py);
		      bm_setpixel(*px,*py+1);
		      bm_setpixel(*px+1,*py+1);
		    }
		  bm_setpixel(*px,*py);
		}
	    }
	  
  	  for (k=0;k<FONT_SIZE_X;++k)
  	    {
  	      if (b<0) /* check sign bit. */
		{
		  /* bm_setforeground(mainColor); */
		  if (ModeBold)
		    {
		      bm_setpixel(*px+1,*py);
		      bm_setpixel(*px,*py+1);
		      bm_setpixel(*px+1,*py+1);
		    }
		  bm_setpixel(*px,*py);
		}
	      else
		{
		  if (CurMode==ModeInverse || CurMode==ModeRewrite)
		    {
		      /* bm_setforeground(altColor); */
		      if (ModeBold)
			{
			  bm_clearpixel(*px+1,*py);
			  bm_clearpixel(*px,*py+1);
			  bm_clearpixel(*px+1,*py+1);
			}
		      bm_clearpixel(*px,*py); 
		    }
		}

	      x += deltaX;
  	      b<<=1;
  	    }

	  y+=deltaY;
	  x-=width;
	  ++p;
  	}

      Coord->x+=width;
      x+=width;
      y-=height;
    }

  return;
  
}

/**
 * screen_tty_char - Called to plot chars when in tty mode
 */
void screen_tty_char(padByte theChar)
{
  if ((theChar >= 0x20) && (theChar < 0x7F)) {
    screen_char_draw(&TTYLoc, &theChar, 1);
    TTYLoc.x += CharWide;
  }
  else if ((theChar == 0x0b)) /* Vertical Tab */
    {
      TTYLoc.y += CharHigh;
    }
  else if ((theChar == 0x08) && (TTYLoc.x > 7))	/* backspace */
    {
      padPt ec1,ec2;
      TTYLoc.x -= CharWide;
      ec1.x=TTYLoc.x;
      ec1.y=TTYLoc.y;
      ec2.x=TTYLoc.x+CharWide;
      ec2.y=TTYLoc.y+CharHigh;
      screen_block_draw(&ec1,&ec2);
   }
  else if (theChar == 0x0A)			/* line feed */
    TTYLoc.y -= CharHigh;
  else if (theChar == 0x0D)			/* carriage return */
    TTYLoc.x = 0;
  
  if (TTYLoc.x + CharWide > 511) {	/* wrap at right side */
    TTYLoc.x = 0;
    TTYLoc.y -= CharHigh;
  }
  
  if (TTYLoc.y < 0) {
    screen_clear();
    TTYLoc.y=495;
  }

}

/**
 * screen_foreground - Called to set foreground color.
 */
void screen_foreground(padRGB* theColor)
{
}

/**
 * screen_background - Called to set foreground color.
 */
void screen_background(padRGB* theColor)
{
}

/**
 * screen_paint - Called to paint at location.
 */
void screen_paint(padPt* Coord)
{
}

/**
 * screen_clear_status(void)
 * Clear status area
 */
void screen_clear_status(void)
{
  padPt coord1={0,0};
  padPt coord2={511,CharHigh};
  screen_block_draw(&coord1,&coord2);
}

/**
 * screen_show_status(msg)
 */
void screen_show_status(unsigned char* msg,int len)
{
  screen_clear_status();
  screen_char_draw(&statusLoc,msg,len);
}

/**
 * screen_show_baud_rate - Show baud rate
 */
void screen_show_baud_rate(void)
{
  screen_clear_status();
  switch(io_baud_rate)
    {
    case BAUD_300:
      screen_show_status("300 baud.",9);
      break;
    case BAUD_1200:
      screen_show_status("1200 baud.",10);
      break;
    case BAUD_2400:
      screen_show_status("2400 baud.",10);
      break;
    case BAUD_4800:
      screen_show_status("4800 baud.",10);
      break;
    case BAUD_9600:
      screen_show_status("9600 baud.",10);
      break;
    case BAUD_19200:
      screen_show_status("19200 baud.",11);
      break;
    case BAUD_38400:
      screen_show_status("38400 baud.",11);
      break;      
    }
}

/**
 * screen_done()
 * Close down TGI
 */
void screen_done(void)
{
}
