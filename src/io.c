#include "io.h"
#include "protocol.h"
#include "vdp.h"
#include "conio.h"
#include "rs232.h"

BaudRate io_baud_rate;

void io_init(void)
{
  rs232_setcontrol(RS232_CARD,RS232_9902A,RS232_CTRL_STOP_1|RS232_CTRL_NO_PARITY|RS232_CTRL_8_BIT);
  rs232_setbps(RS232_CARD,RS232_9902A,RS232_BPS_1200);
  io_baud_rate=BAUD_1200;
}

void io_set_baud_rate(void)
{
  int new_baud_rate;
  switch(io_baud_rate)
    {
    case BAUD_300:
      new_baud_rate=RS232_BPS_300;
      break;
    case BAUD_1200:
      new_baud_rate=RS232_BPS_1200;
      break;
    case BAUD_2400:
      new_baud_rate=RS232_BPS_2400;
      break;
    case BAUD_4800:
      new_baud_rate=RS232_BPS_4800;
      break;
    case BAUD_9600:
      new_baud_rate=RS232_BPS_9600;
      break;
    case BAUD_19200:
      new_baud_rate=RS232_BPS_19200;
      break;
    case BAUD_38400:
      new_baud_rate=RS232_BPS_38400;
      break;
    }
  rs232_setbps(RS232_CARD,RS232_9902A,new_baud_rate);
}

void io_toggle_baud_rate(void)
{
  if (io_baud_rate==BAUD_38400)
    io_baud_rate=BAUD_300;
  else
    io_baud_rate++;
  io_set_baud_rate();
}

void io_send_byte(int b)
{
  rs232_writebyte(RS232_CARD,RS232_9902A,b);
}

void io_main(void)
{
  int ib;
  unsigned char b;
  if (rs232_poll(RS232_CARD,RS232_9902A))
    {
      ib=rs232_readbyte(RS232_CARD,RS232_9902A);
      b=ib;
      ShowPLATO(&b,1);
    }
}

void io_done()
{
}
