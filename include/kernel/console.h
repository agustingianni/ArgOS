#ifndef CONSOLE_H_
#define CONSOLE_H_

#define NCOLS			80
#define NROWS			25
#define ATTRIBUTE		7
#define VIDEO_MEM_ADDR	(unsigned short *) 0xB8000

#define TABWIDTH		8

#define CRT_ADDR_REG 			0x3D4
#define CRT_DATA_REG 			0x3D5
#define CRT_CURSOR_LOC_HIGH_REG 0x0E
#define CRT_CURSOR_LOC_LOW_REG 	0x0F

#define BLACK   		0
#define BLUE    		1
#define GREEN   		2
#define CYAN    		3
#define RED     		4
#define MAGENTA 		5
#define AMBER   		6
#define GRAY    		7
#define BRIGHT  		8
#define RESET			9

#define ATTRIB(bg,fg) 	((fg & 0x0F) | ((bg) << 4))

/* TODO: Implement this */
typedef struct screen
{
	unsigned short *video_address;	/* Video address 		*/
	unsigned char attributes;		/* Console attributes 	*/
	int x;							/* Column 				*/
	int y;							/* Row 					*/
} console_t;


void screen_get_cursor (int* row, int* col);
void screen_set_cursor (int row, int col);
unsigned char  screen_get_attrib (void);
void screen_set_attrib (unsigned char attrib);

void screen_clear (void);
int putchar	(int c);
 
#endif /*CONSOLE_H_*/
