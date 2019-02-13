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
unsigned char current_foreground=COLOR_WHITE;
unsigned char current_background=COLOR_BLACK;
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

#define FONTPTR(a) (a*6)

/**
 * sndplay(sndlist, len) - Copy sound list into VDP and play
 */
void sndplay(unsigned char* sndlst, int len)
{        
    int vaddr = 0x3f00; // address of a vdp buffer you know of, to put the sound list into

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
  set_bitmap(VDP_SPR_16x16);
  bm_setbackground(current_background);
  bm_setforeground(current_foreground);
  bordercolor(current_background);
  bgcolor(current_background);
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
  bm_setbackground(current_background);
  bm_setforeground(current_foreground);
}

/**
 * screen_block_draw(Coord1, Coord2) - Perform a block fill from Coord1 to Coord2
 */
void screen_block_draw(padPt* Coord1, padPt* Coord2)
{
  int mode;
  int x1=min(scalex(Coord1->x),scalex(Coord2->x));
  int x2=max(scalex(Coord1->x),scalex(Coord2->x));
  int y1=min(scaley(Coord1->y),scaley(Coord2->y));
  int y2=max(scaley(Coord1->y),scaley(Coord2->y));
  int y;
  
  screen_set_pen_mode();

  for(y=y1;y<y2;y++)
    bm_clearhlinefast(x1,y,x2);
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
    bm_drawlinefast(x1,y1,x2,y2,1);
  else
    bm_drawlinefast(x1,y1,x2,y2,0);

}

/**
 * screen_char_draw(Coord, ch, count) - Output buffer from ch* of length count as PLATO characters
 */
void screen_char_draw(padPt* Coord, unsigned char* ch, unsigned char count)
{
  short offset; /* due to negative offsets */
  int x;      /* Current X and Y coordinates */
  int y;
  int* px;   /* Pointers to X and Y coordinates used for actual plotting */
  int* py;
  unsigned char i; /* current character counter */
  unsigned char a; /* current character byte */
  unsigned char j,k; /* loop counters */
  char b; /* current character row bit signed */
  unsigned char width=FONT_SIZE_X;
  unsigned char height=FONT_SIZE_Y;
  unsigned short deltaX=1;
  unsigned short deltaY=1;
  unsigned char mainColor=current_foreground;
  unsigned char altColor=current_background;
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
      altColor=current_background;
    }
  else if (CurMode==ModeInverse)
    {
      altColor=current_foreground;
    }
  
  if (CurMode==ModeErase || CurMode==ModeInverse)
    mainColor=current_background;
  else
    mainColor=current_foreground;

  bm_setforeground(mainColor);
  
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
	      if (x>255)
		x=x-256;
  	      b<<=1;
  	    }

	  ++y;
	  if (y>191)
	    y=y-192;
	  x-=width;
	  /* if (x<0) */
	  /*   x=256+x; */
	  ++p;
  	}

      x+=width;
      if (x>255)
	x=x-256;
      y-=height;
      if (y>191)
	y=y-192;
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
	      if (x>255)
		x=x-256;
  	      b<<=1;
  	    }

	  y+=deltaY;
	  if (y>192)
	    y=y-192;
	  x-=width;
	  if (x<0)
	    x=256+x;
	  ++p;
  	}

      Coord->x+=width;
      x+=width;
      if (x>256)
	x=x-256;
      y-=height;
      if (y>192)
	y=y-192;
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
 * screen_color - return closest match to requested color.
 */
unsigned char screen_color(padRGB* theColor)
{
  unsigned char red=theColor->red;
  unsigned char green=theColor->green;
  unsigned char blue=theColor->blue;

  if (red==0 && green==0 && blue==0)
    {
      current_foreground=COLOR_BLACK;
    }
  else if (red==0 && green==0 && blue==255)
    {
      current_foreground=COLOR_LTBLUE;
    }
  else if (red==0 && green==255 && blue==0)
    {
      current_foreground=COLOR_LTGREEN;
    }
  else if (red==255 && green==0 && blue==0)
    {
      current_foreground=COLOR_LTRED;
    }
  else if (red==0 && green==255 && blue==255)
    {
      current_foreground=COLOR_CYAN;
    }
  else if (red==255 && green==0 && blue==255)
    {
      current_foreground=COLOR_MAGENTA;
    }
  else if (red==255 && green==255 && blue==0)
    {
      current_foreground=COLOR_LTYELLOW;
    }
  else
    {
      current_foreground=COLOR_WHITE;
    }
}


/**
 * screen_foreground - Called to set foreground color.
 */
void screen_foreground(padRGB* theColor)
{
  current_foreground=screen_color(theColor);
}

/**
 * screen_background - Called to set foreground color.
 */
void screen_background(padRGB* theColor)
{
  /* current_background=screen_color(theColor); */
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
