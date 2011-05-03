#ifndef RTC_H
#define RTC_H

#include <types.h>
#include <drivers/io.h>

//! The real-time clock interrupt.
#define RTC_IRQ			8

//! The base epoch year.
#define	RTC_EPOCH_YEAR		1970
//! The base epoch day (Jan 1).
#define	EPOCH_DAY		0

//! RTC port for seconds.
#define RTC_SECONDS		0
//! RTC port for minutes.
#define RTC_MINUTES		2
//! RTC port for hours.
#define RTC_HOURS		4

//! RTC port for the day of the week.
#define RTC_DAY_OF_WEEK		6
//! RTC port for the month.
#define RTC_DAY_OF_MONTH	7
//! RTC port for month.
#define RTC_MONTH		8
//! RTC port for year.
#define RTC_YEAR		9

//! RTC control register A.
#define RTC_REG_A		10
//! RTC control register B.
#define RTC_REG_B		11
//! RTC control register C.
#define RTC_REG_C		12
//! RTC control register D.
#define RTC_REG_D		13

//! Macro to convert from BCD to binary format.
#define BCD2BIN(n) ( ((n) >> 4) * 10 + ((n) & 0x0F) )
//! Macro to convert from binary to BCD format.
#define BIN2BCD(n) ( (((n) / 10) << 4) + ((n) % 10) )

//! CMOS control register.
#define CMOS_CTRL		0x70
//! CMOS data register.
#define CMOS_DATA		0x71

//! \brief Read a value from the CMOS.
//! \param port The port to read from.
//! \return The value read from the CMOS \a port.
#define CMOS_READ(port) ( \
		{ \
			outb( CMOS_CTRL, (port) ); \
			inb( CMOS_DATA ); \
		})

//! \brief Write a value to the CMOS.
//! \param port The port to write to.
//! \param val The value to write.
#define CMOS_WRITE(port, val) ( \
		{ \
			outb( CMOS_CTRL, (port) ); \
			outb( CMOS_DATA, (val) ); \
		})

// --- Register Details ----------------------------------------------- //
// ******************************************************************** //
//! RTC update frequency register.
#define RTC_FREQ_SELECT		RTC_REG_A

//! Update-in-progress - set to "1" 244 microsecs before RTC goes off
//! the bus, reset after update (may take 1.984ms @ 32768Hz RefClock)
//! is complete, totalling to a max high interval of 2.228 ms.
#define RTC_UIP		0x80

#define RTC_DIV_RESET1	0x60
#define RTC_DIV_RESET2	0x70

//! Periodic interrupt enable.
#define RTC_PIE		0x40

// ******************************************************************** //
//! RTC status register.
#define RTC_CONTROL		RTC_REG_B

//! Disable updates for clock setting.
#define RTC_SET		0x80
//! RTC binary date; all time/date values are BCD if clear.
#define RTC_DM_BINARY	0x04
// 24 hour mode - else hours bit 7 means pm.
#define RTC_24H		0x02

//! Time values returned by the CMOS real-time clock.
typedef struct rtc_time
{
	unsigned int dayweek : 3;
	unsigned int daymon : 5;
	unsigned char mon;
	unsigned short year;
	unsigned char sec;
	unsigned char min;
	unsigned char hour;
} __attribute__ ((packed)) rtc_time_t;

int time_read( rtc_time_t *rtc );
int time_write( rtc_time_t *rtc );
int init_rtc(void);

//time_t sys_time(time_t *t);

#endif /* RTC_H */
