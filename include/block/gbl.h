/*Generic Block Layer*/
#ifndef GBL_H_
#define GBL_H_

#include <block/bio.h>

#define KERNEL_SECTOR_SIZE 512

/*Posibles errores durante la operacion*/
#define OUT_OF_RANGE 0

extern struct disk_set all_disks;

/*Funcion RAW utilizada para hacer peticiones
  de bloques a un dispositivo.*/
void make_request(struct bio *);

/*Esta funcion devuelve un puntero a una
  estructura gendisk, la cual hay que
  completar con la informacion del
  disco a registrar y luego llamar a
  register_disk para finalizar la operacion.*/
struct gendisk * alloc_disk(void);

/*Confirma que el dato pasado como parametro
  quede registrado como un disco generico.*/
void   register_disk(struct gendisk *);

struct block_device * alloc_blkdev();
void register_blkdev(struct block_device *);
#endif
