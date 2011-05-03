#ifndef GENHD_H_
#define GENHD_H_

#include <types.h>
#include <block/request.h>
#include <block/block_device.h>

/*Particiones dentro de la unidad*/
struct hd_struct {
    sector_t start_sect;
    sector_t nr_sects;
    __u32    ios[2], sectors[2];          /*leidos y escritos*/
    __u32    policy, partno;
};

/*Para representar dispositivos de bloque en general*/
struct gendisk {
    __u32 major;                          /*major number of driver */
    __u32 minors;                         /*maximum number of minors, =1 for
                                            disks that can't be partitioned.*/
    __u32 first_minor;

    char  disk_name[32];                  /*name of major driver */
    struct hd_struct **part;              /*vector de particiones
                                            [indexed by minor] */
    struct block_device_operations *fops; /*funciones de control*/
    struct request_queue queue;           /*cola de peticiones*/
    void *private_data;                   /*datos privados del driver*/
    sector_t capacity;                    /*capacidad del disco, en sectores*/

    __u32 flags;
};

#endif
