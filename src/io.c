#include "io.h"
#include "protocol.h"
#include "vdp.h"
#include "conio.h"
#include "rs232.h"

void io_init(void)
{
  rs232_setcontrol(RS232_CARD,RS232_9902A,RS232_CTRL_STOP_1|RS232_CTRL_NO_PARITY|RS232_CTRL_8_BIT);
  rs232_setbps(RS232_CARD,RS232_9902A,RS232_BPS_1200);
}

void io_send_byte(int b)
{
  rs232_writebyte(RS232_CARD,RS232_9902A,b);
}

void io_send_byte1(int b)
{
  rs232_writebyte(RS232_CARD,RS232_9902A,b>>8);
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
