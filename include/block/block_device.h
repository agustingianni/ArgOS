#ifndef BLOCK_DEVICE_H_
#define BLOCK_DEVICE_H_

#include <types.h>

/*Representamos todos los dispositivos de bloque que estan
  siendo usados en el sistema.*/
struct block_device
{
    struct gendisk *disk;
    __u32           init_seg;
    __u32           size;

    __u32           minor;
    /*...*/
};

struct block_device_operations
{
    __u32 (*open)   (void);
    __u32 (*release)(void);
    //ioctl y demas funciones
};

#endif
