#include <kernel/pageswapper.h>

/*
 */
int
page_out_scan(void)
{
	return -1;
}

/*
 * De a cuerdo a cuanta memoria libre haya este "daemon"
 * va a llamar a page_out_scan() para que escanee las paginas
 * que van a ser swapeadas al disco.
 * 
 * El parametro de cuando llamar a page_out_scan va a ser
 * calibrado de acuerdo a la carga del sistema etc.
 */
int
page_out_daemon(void)
{
	return -1;
}
