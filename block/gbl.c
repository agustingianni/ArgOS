#include <block/block_device.h>
#include <block/gbl.h>
#include <block/request.h>
#include <block/genhd.h>

/*disk_set es la variable que contiene
  las referencias a los dispositivos
  de almacenamiento registrados.*/
struct disk_set{
    struct gendisk disks[8];
    __u16  first;             /*Primer registro libre*/
    //atomic_t mutex;
}all_disks;

struct block_device block_devices[16];
static unsigned int blkdev_count = 0;

void make_request(struct bio * block)
{
    /*Colocamos la direccion del primer segmento en
      relacion al comienzo del disco y no al de la
      particion (si la tiene).*/
    __u32 offset = 0;
    struct request_queue *queue;
    struct block_device  *device = block->bi_bdev;

    offset = device->init_seg;
    /*Se puede optimizar la division con >>*/
    /*Esto hay que mejorarlo*/
    if ((block->bi_size/KERNEL_SECTOR_SIZE)\
         > device->size)
    {
        /*En caso de verse superado este limite,
          se llama a la funcion de terminacion de
          la operacion, adjuntando el error.*/
        block->bi_end_io(block, block->bi_size, OUT_OF_RANGE);
        return;
    }

    /*Sumamos el offset de comienso de la particion.*/
    block->bi_sector += offset;

    /*Para despachar la operacion al dispositivo
      correspondiente, agregamos el bio a la
      request_queue, atravez del algoritmo elevador
      (I/O Scheaduler) que se encargara de encadenarla
      a un request existente o crear uno nuevo.*/

    queue = &(device->disk->queue);
    queue->schedul_request(queue, block);
}

struct gendisk * alloc_disk(void)
{
    /*Mas adelante se usara kmalloc en lugar
      de pasar una referencia.*/
    return &(all_disks.disks[all_disks.first]);
}

void register_disk(struct gendisk *disk)
{
    /*Por ahora solo incrementamos el numero
      de discos registrados, pero la idea es
      tener una lista enlasada de discos mas
      adelante.*/
    all_disks.first++;
}

struct block_device * alloc_blkdev()
{
    return &(block_devices[blkdev_count]);
}

void register_blkdev(struct block_device *blkdev)
{
    blkdev_count++;
}


