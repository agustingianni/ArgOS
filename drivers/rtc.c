#include <kernel/console.h>
#include <kernel/time.h>
#include <lib/time.h>
#include <arch/irq.h>

int time_read( rtc_time_t *rtc )
{
	int i;

	// When the Update-In-Progress (UIP) flag goes from 1 to 0,
	// the RTC registers show the second which has precisely just
	// started.

	// Read RTC exactly on falling edge of update flag.
	for( i = 0; i < 1000000 ; i++ )	// may take up to 1 second...
		if ( CMOS_READ(RTC_FREQ_SELECT) & RTC_UIP )
			break;

	// Must try at least 2.228 ms
	for( i = 0 ; i < 1000000 ; i++ )
		if ( !(CMOS_READ(RTC_FREQ_SELECT) & RTC_UIP))
			break;

	// Read data from the CMOS.
	rtc->sec = BCD2BIN( CMOS_READ(RTC_SECONDS) );
	rtc->min = BCD2BIN( CMOS_READ(RTC_MINUTES) );
	rtc->hour = BCD2BIN( CMOS_READ(RTC_HOURS) );
	rtc->dayweek = BCD2BIN( CMOS_READ(RTC_DAY_OF_WEEK) );
	rtc->daymon = BCD2BIN( CMOS_READ(RTC_DAY_OF_MONTH) );
	rtc->mon = BCD2BIN( CMOS_READ(RTC_MONTH) );
	rtc->year = BCD2BIN( CMOS_READ(RTC_YEAR) );

	if( (rtc->year += 1900) < 1970 )
	{
		rtc->year += 100;
	}

	return( 0 );
}

//! Initialize the real-time clock to generate periodic interrupt.
// \warning Unused for now...
int init_rtc( void )
{
	uint8_t status_b;

	int iflags = begin_atomic();
	{
		// Install the irq handler.
		// install_trap_handler( RTC_IRQ, &time_handler );
	
		// Read status registers.
		status_b = CMOS_READ( RTC_REG_B );
		// Enable periodic interrupt.
		status_b |= RTC_PIE;
	
		// Write status registers.
		CMOS_WRITE( RTC_REG_B, status_b );
	}
	end_atomic(iflags);

	return( 0 );
}

/* Get the seconds passed since midnight 1970-01-01 */
time_t sys_time( time_t *t )
{
	rtc_time_t rtc;
	struct tm tm;
	time_t ret = (time_t) (-1);

	if( time_read( &rtc ) == 0 )
	{
		// Copy real time clock values into the standard
		// structure.
		tm.tm_year = rtc.year;
		tm.tm_mon = rtc.mon;
		tm.tm_mday = rtc.daymon;
		tm.tm_hour = rtc.hour;
		tm.tm_min = rtc.min;
		tm.tm_sec = rtc.sec;

		// Make the UNIX timestamp.
		ret = mktime( &tm );
	}
	if( t != NULL )
	{
		// Update the argument.
		*t = ret;
	}

	return( ret );
}
