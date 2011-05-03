#include <kernel/console.h>
#include <lib/string.h>
#include <drivers/io.h>
#include <assert.h>

static console_t console;
static unsigned short *console_addr;
static unsigned char console_attrib;
static int console_x 		= 0;
static int console_y 		= 0;

/*
 * Scroll down one line the screen
 */
void screen_scroll(void)
{
    unsigned temp;

    /* Check if we really need to scroll */
    if(console_y >= NROWS)
    {
    	/* Calculate the number of rows we need to copy */
        temp = console_y - NROWS + 1;
        memmove (console_addr, console_addr + temp * NCOLS,
        		(NROWS - temp) * NCOLS * 2);
        
        /* we use memset with short argument to write two bytes */
        memsetw(console_addr + (NROWS - temp) * NCOLS, ' ', NCOLS);
        console_y = NROWS - 1;
    }
}

/* 
 * Update the hardware cursor
 */
void screen_update_cursor(void)
{
    unsigned temp = console_y * NCOLS + console_x;

    outb(CRT_ADDR_REG, 14);
    outb(CRT_DATA_REG, temp >> 8);
    outb(CRT_ADDR_REG, 15);
    outb(CRT_DATA_REG, temp);
}

/*
 * Clear the screen
 */
void screen_clear(void)
{
	int i;

    for(i = 0; i < NROWS; i++)
    {
    	memsetw(console_addr + i * NCOLS, ' ', NCOLS);
    }

    console_x = 0;
    console_y = 0;
    
    screen_update_cursor();
}

/* Puts a single character on the screen */
int putchar(int c)
{
    unsigned att = console_attrib << 8;

    switch(c)
    {
    	case 0x08:			/* backspace */
    	{
	        if(console_x)
	        {
	        	console_x--;
	        }
	        break;
    	}
    	case 0x09:			/* TAB */
    	{
    		console_x += (TABWIDTH - (console_x % TABWIDTH));
    		break;
    	}
    	case '\r':			/* charriage return */
	    {
	    	console_x = 0;
	    	break;
	    }
    	case '\n':			/* New line */
	    {
	        console_x = 0;
	        console_y++;
	        break;
	    }
    	default:			/* Printable character */
    	{
    		/* Write the character */
            *(console_addr + (console_y * NCOLS + console_x)) = (c | att);
            console_x++;
            break;
    	}
    }

    if(console_x >= NCOLS)
    {
        console_x = 0;
        console_y++;
    }

    /* Scroll if needed */
    screen_scroll();
    screen_update_cursor();
    
    return 0;
}

/* Uses the above routine to output a string... */
void puts(const char *s)
{
    int i;

    for (i = 0; i < strlen(s); i++)
    {
        putchar(s[i]);
    }
}

void screen_get_cursor (int *row, int *col)
{
	assert(row != NULL && col != NULL);
	*row = console_y;
	*col = console_x; 
}

/*
 * TODO: Lock
 */
void screen_set_cursor (int row, int col)
{
    assert(row >= 0 && row < NROWS && col >= 0 && col < NCOLS);

    console_y = row;
    console_x = col;
    
    screen_update_cursor();
}

unsigned char screen_get_attrib (void)
{
	return console_attrib;
}

/*
 * TODO: Lock
 */
void screen_set_attrib (unsigned char attrib)
{
	console_attrib = attrib;
}

/*
 * Sets the forecolor and backcolor that we will use
 */
void screen_set_color(unsigned char bg, unsigned char fg)
{
	screen_set_attrib(ATTRIB(bg, fg));
}

/*
 * Initialize VGA
 */
void init_video(void)
{
	console.video_address = VIDEO_MEM_ADDR;
	console_addr = VIDEO_MEM_ADDR;
	
    screen_set_color(BLACK, BRIGHT | GREEN);
    screen_clear();
}
