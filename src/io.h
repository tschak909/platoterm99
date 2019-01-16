/**
 * PLATOTerm64 - A PLATO Terminal for the Commodore 64
 * Based on Steve Peltz's PAD
 * 
 * Author: Thomas Cherryhomes <thom.cherryhomes at gmail dot com>
 *
 * io.h - Input/output functions (serial/ethernet)
 */

#ifndef IO_H
#define IO_H

#define XON  0x11
#define XOFF 0x13

typedef enum _baudRate
  {
   BAUD_300,
   BAUD_1200,
   BAUD_2400,
   BAUD_4800,
   BAUD_9600,
   BAUD_19200,
   BAUD_38400
  } BaudRate;

/**
 * io_init() - Set-up the I/O
 */
void io_init(void);

/**
 * io_init_funcptrs() - Set up I/O function pointers
 */
void io_init_funcptrs(void);

/**
 * io_open() - Open the device
 */
void io_open(void);

/**
 * io_set_baud_rate(void)
 * Set new baud rate
 */
void io_set_baud_rate(void);

/**
 * io_toggle_baud_rate(void)
 * Set next baud rate.
 */
void io_toggle_baud_rate(void);

/**
 * io_send_byte(b) - Send specified byte out
 */
void io_send_byte(int b);

/**
 * io_main() - The IO main loop
 */
void io_main(void);

/**
 * io_recv_serial() - Receive and interpret serial data.
 */
void io_recv_serial(void);


/**
 * io_done() - Called to close I/O
 */
void io_done(void);

#endif /* IO_H */
