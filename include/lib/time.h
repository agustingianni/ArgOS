#ifndef TIME_H_
#define TIME_H_

#define isleap(y) ((((y) % 4) == 0 && ((y) % 100) != 0) || ((y) % 400) == 0)

//! Time structure.
struct tm
{
	int tm_sec;	//!< Seconds.
	int tm_min;	//!< Minutes.
	int tm_hour;	//!< Hours.
	int tm_mday;	//!< Day of the month.
	int tm_mon;	//!< Month.
	int tm_year;	//!< Year.
	int tm_wday;	//!< Day of the week.
	int tm_yday;	//!< Day in the year.
	int tm_isdst;	//!< Daylight saving time.
};

//! Structure to specify intervals of time with nanosecond pecision.
struct timespec
{
	time_t	tv_sec;		//!< seconds
	long	tv_nsec; 	//!< nanoseconds
};

static inline time_t mktime( struct tm *tm )
{
	/* 1..12 -> 11,12,1..10 */
	if (0 >= (int) (tm->tm_mon -= 2)) {
		/* Puts Feb last since it has leap day */
		tm->tm_mon += 12;
		tm->tm_year -= 1;
	}

	return (((
		(unsigned long) (tm->tm_year/4 - tm->tm_year/100 + tm->tm_year/400 + 367*tm->tm_mon/12 + tm->tm_mday) +
			tm->tm_year*365 - 719499
	    )*24 + tm->tm_hour /* now have hours */
	  )*60 + tm->tm_min /* now have minutes */
	)*60 + tm->tm_sec; /* finally seconds */
}

#endif /*TIME_H_*/
