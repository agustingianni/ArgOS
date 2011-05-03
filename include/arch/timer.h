#ifndef TIMER_H_
#define TIMER_H_

 /* the 8254 chip's internal oscillator frequency. */
 #define CLOCK_TICK_RATE 1193182 

 /*
  * We are going to generate an IRQ every millisecond
  * This establishes the timer interrupt frequency
  */
 #define HZ 1000

 /**/
 #define LATCH	(CLOCK_TICK_RATE / HZ)

 /*
  * @doc:
  * http://en.wikipedia.org/wiki/Intel_8253
  * http://www.osdever.net/bkerndev/Docs/pit.htm
  * 
  * Control Word Format:
  * 
  * Bit#   D7   D6    D5   D4     D3  D2  D1    D0
  * Name   SC1  SC0   RW1  RW0    M2  M1  M0    BCD
  * ----  --------   ----------  -----------   ------------------------
  * Func.  Select     Read/Write  Select        =0, 16-b binary counter
  *        Counter                Mode          =1, 4-decade BCD counter
  */

 #define PIT_COUNTER_0 			(0 << 6) 
 #define PIT_COUNTER_1 			(1 << 6)
 #define PIT_COUNTER_2 			(2 << 6)

 /* 
  * The Control Word Register can only be written to;
  * status information is available with the Read-Back Command.
  */
 #define PIT_CTRL_WR   			(3 << 6)

 /*
  * The IO addreses of the PIT registers (aka. the pit's channels) 
  */

 #define PIT_BASE_ADDRESS 		0x40
 #define PIT_COUNTER_0_ADDR 	PIT_BASE_ADDRESS + PIT_COUNTER_0
 #define PIT_COUNTER_1_ADDR 	PIT_BASE_ADDRESS + PIT_COUNTER_1
 #define PIT_COUNTER_2_ADDR 	PIT_BASE_ADDRESS + PIT_COUNTER_2
 #define PIT_CTRL_WR_ADDR   	PIT_BASE_ADDRESS + PIT_CTRL_WR


 /*
  * RW Read/Write:
  * RW1 RW0
  * 0   0 Counter Latch Command (see Read Operations)
  *       
  *        
  * 0   1 Read/Write least significant byte only.
  * 1   0 Read/Write most significant byte only.
  * 1   1 Read/Write least significant byte first, then most significant byte.
  */

 #define PIT_CMD_LATCH			(0 << 4)
 #define PIT_CMD_RW_LSB			(1 << 4)
 #define PIT_CMD_RW_MSB			(2 << 4)
 #define PIT_CMD_RW_LSB_MSB		(3 << 4)

 /*
  * M Mode
  * 0 0 0 = Mode 0 (interrupt on terminal count)
  * 0 0 1 = Mode 1 (hardware re-triggerable one-shot)
  * 0 1 0 = Mode 2 (rate generator)
  * 0 1 1 = Mode 3 (square wave generator)
  * 1 0 0 = Mode 4 (software triggered strobe)
  * 1 0 1 = Mode 5 (hardware triggered strobe)
  * 1 1 0 = Mode 6 (rate generator, same as 010b)
  * 1 1 1 = Mode 7 (square wave generator, same as 011b)
  */

 #define PIT_CMD_MODE_0			(0 << 1)
 #define PIT_CMD_MODE_1			(1 << 1)
 #define PIT_CMD_MODE_2			(2 << 1)
 #define PIT_CMD_MODE_3			(3 << 1)
 #define PIT_CMD_MODE_4			(4 << 1)
 #define PIT_CMD_MODE_5			(5 << 1)

 /*
  * BCD:
  * 0    Binary Counter 16-bits
  * 1    Binary Coded Decimal (BCD) Counter (4 Decades)
  */

 #define PIT_CMD_COUNT_BINARY	(0 << 0)
 #define PIT_CMD_COUNT_BCD 		(1 << 0)

 void init_timer(void);

#endif /*TIMER_H_*/
