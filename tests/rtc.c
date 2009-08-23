#include <sys/io.h>
#include <unistd.h>
#include <stdio.h>

unsigned char  cmos[10];
char *day[] = { "", "MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN" };
char *month[] = { "", "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
			"JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };

int
main(int argc, char **argv)
{
	int i, len = 0;

	if(ioperm(0x70, 2, 1) == -1)
	{
		perror("ioperm()");
		return -1;
	}

	for (i = 0; i < 10; i++)
	{
		outb( i, 0x70 );
		cmos[i] = inb(0x71);
	}

	// show the current time and date
	printf("CMOS Real-Time Clock/Calendar:" );
	printf(" %02X", cmos[4]);	// current hour
	printf(":%02X", cmos[2]);	// current minutes
	printf(":%02X", cmos[0]);	// current seconds
	printf(" on");
	printf(" %s, ", day[cmos[6]]); 	// day-name
	printf("%02X", cmos[7]);		// day-number
	printf(" %s", month[cmos[8]]);	// month-name
	printf(" 20%02X ", cmos[9]);		// year-number
	printf("\n" );

	return	0;
}
