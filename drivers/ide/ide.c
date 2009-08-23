#include <drivers/ide/ata.h>
#include <drivers/ide/ide.h>
#include <block/gbl.h>
#include <block/genhd.h>
#include <block/request.h>
#include <drivers/io.h>
#include <jiffies.h>
#include <arch/timer.h>
#include <arch/delay.h>
#include <lib/vsprintf.h>
#include <lib/string.h>


#include "../include/kernel/printk.h"

/*Aqui se implementa lo que se considera un
  controlador generico de dispositivos ide
  (teniendo en cuenta con esto solo los discos).
  Una explicacion de como los discos se conectan
  al sistema.
  Cada dispositivo, ya sea un disco, CDROM, etc
  es manejado por una electronica que llamaremos
  driver de aqui en mas.
  Cada uno de estos drivers esta conectado al bus no
  directamente, sino atravez de un Controlador, que
  es quien mapea sus registros en los puertos de E/S.
  Con estos nombres conoceremos a estas partes en
  el codigo.*/

static struct ide_io_port ctrls[] = {{0x1f0, 0x3f6, 14, 0},\
                                     {0x170, 0x376, 15, 1},\
                                     {0x1e8, 0x3ee, 11, 2},\
                                     {0x168, 0x36e, 10, 3}};

/*Estado de las operaciones para cada una
  de las posibles controladoras del sistema.*/
/*0xa0 pone los primeros 3 bits mas
  significativos a 111, que son valores
  usados por defecto en los controladores.
*/
static struct ctrl_operation operations[] = {{NULL, LBA_DRIVE_BITS_MASK},\
                                             {NULL, LBA_DRIVE_BITS_MASK},\
                                             {NULL, LBA_DRIVE_BITS_MASK},\
                                             {NULL, LBA_DRIVE_BITS_MASK}};
/*El numero de controladores que este driver soporta*/
#define CTRLS_AMOUNT 4

static struct {
    /*Numero de discos detectados*/
    unsigned int count;
    struct {
        /*Indice en ctrls*/
        unsigned int   io_port_index;
        /*Indica si es master o slave*/
        unsigned short unit;
    }data[8];
}detected_disks;

__u8 copy_from_io(struct ide_io_port *dev,\
                  unsigned char      *buffer,\
                  unsigned int        count)
{
    unsigned int words  = count*SECTOR_SIZE/2;
    unsigned short port = dev->command+CMD_DATA_OFFT;

    if (!((inb(dev->control + CTR_ALT_STATUS_OFFT)) & STS_DRQ))
        return 0;

    /*Leemos del registro de datos*/
    insw(port, (void *)buffer, words);

    return count;
}

void short_to_string(__u8 *string, __u16 size)
{
    __u16 i = 0;
    __u8 tmp;

    for (; i < size; i+=2)
    {
        if ((string[i] == string[i+1])&&(string[i] == ' '))
        {
            i++;
            break;
        }
        tmp = string[i];
        string[i] = string[i+1];
        string[i+1] = tmp;
    }

    string[i-1] = '\0';
}

void get_ata_info(struct ata_params  *data,\
                  struct ide_io_port *ctrl,\
                  unsigned char       unit)
{
    /*Seleccionamos la unidad*/
    SELECT_DRIVER((*ctrl),unit);

    /*Escribimos el comando en el registro*/
    do_command(ctrl, ATA_ATA_IDENTIFY, 0);

    copy_from_io(ctrl, (unsigned char *)data, 1);

    /*Estas funciones ponen las cadenas leidas de
      a palabras (16bits), en orden.*/
    short_to_string(data->serial,   20);
    short_to_string(data->revision, 8);
    short_to_string(data->model,    40);
}

void bottom_half()
{
    //struct bio *opt = operations[index].operation;
    //copy_from_io(&(ctrls[index]), (unsigned char *)(opt->bi_io_vec->data), 1);
}

void ide_handler(icontext_t *ctx)
{
    __u32 index = get_index_from_irq(ctx->int_no);
    struct bio *opt = operations[index].operation;
    //printk("ide irq: %d, index: %d\n", ctx->int_no, index);

    /*Si fue una operacion de lectura la realizada, llamamos
      llamamos al bottom_half, sino no.*/
    if ((opt->bi_rw) & BIO_RW_BIT)
        return;
    /*En este bloque colocamos lo que se supone que correria
      el bottom_half.*/
    {
        //index es el dato pasado al tasklet.
        struct bio *opt = operations[index].operation;
        copy_from_io(&(ctrls[index]), (unsigned char *)(opt->bi_io_vec->data), 1);
    }

    printk("index = %d --- 0x%.2x\n", index, operations[index].driver);
}

__u32 controler_exist(struct ide_io_port *ctrl)
{
    unsigned int port = (ctrl->command + CMD_SECTOR_NUM_OFFT);

    outb(port, MAGIC_NUMBER);

    /*esperamos 4ms masomenos*/
    busy_delay_ticks(HZ/250);
    //{
   //     unsigned int end = (jiffies + HZ/250);

   //     while (jiffies < end);
   // }

    if ((inb(port)) == MAGIC_NUMBER)
        return 1;
    return 0;
}

__u32 have_drive(struct ide_io_port *ctrl, __u8 unit)
{
    SELECT_DRIVER((*ctrl), unit);

    FLUSH_HEDR_REG((*ctrl));

    /*esperamos 4ms masomenos*/

    {
        unsigned int end = (jiffies + HZ/250);

        while (jiffies < end);
    }

    if ((inb(ctrl->control + CTR_ALT_STATUS_OFFT)) & STS_DRDY)
        return 1;
    return 0;
}

void init_disk_reg(struct ide_io_port *ctrl,\
                                 __u32 index,\
                                 __u8  unit)
{
    struct gendisk *tmp_gdisk;
    struct block_device *tmp_blkdev;
    struct ata_params info;
    char unit_leter = 'a' + (index*2);
    unsigned int lba_size, data;

    if (unit)
        unit_leter++;

    /*Obtenemos informacion sobre el disco*/
    get_ata_info(&info, ctrl, unit);

    tmp_gdisk = alloc_disk();

    sprintf(tmp_gdisk->disk_name, "hd%c", unit_leter);
    ATA_TYPE_TO_INDEX((info.config & ATA_ATAPI_TYPE_MASK), data);
    printk("    %s: %s %s, ATA %s drive\n", tmp_gdisk->disk_name,\
                                            info.revision,\
                                            info.model,\
                                            ata_types[data]);

    lba_size = (((int)info.lba_size_2) << 16) | \
                      info.lba_size_1;
    SECTOR_TO_MEGA(lba_size,data);
    printk("    %s: %d sectors (%d MB)\n", tmp_gdisk->disk_name,\
                                   lba_size, data);

    /*Por ahora usamos estos valores para el major
      y el minor, almenos asta implementar syscalls
      y que estos datos precisen ser validos.*/
    tmp_gdisk->major       = 0;
    tmp_gdisk->minors      = 0;
    tmp_gdisk->first_minor = 0;

    tmp_gdisk->capacity = lba_size;

    /*Las particiones se detectan luego por otra
      funcion, fuera de la inicializacion de los
      discos.*/
    tmp_gdisk->part = NULL;

    /*Guardamos informacion que va a ser
      necesario conocer durante una operacion.*/
    tmp_gdisk->private_data = (void *)ctrl;

    register_disk(tmp_gdisk);

    /*Registramos el block_device para este disco.*/

    tmp_blkdev = alloc_blkdev();

    tmp_blkdev->disk     = tmp_gdisk;
    tmp_blkdev->init_seg = 0;
    tmp_blkdev->size     = lba_size;
    tmp_blkdev->minor    = 0;

    register_blkdev(tmp_blkdev);
}

__u32  wakeup_disks(void)
{
    unsigned int index = 0;

    /*Iniciamos el numero de discos detectados a 0*/
    detected_disks.count = 0;

    for (; index < CTRLS_AMOUNT; index++)
    {
        if (controler_exist(&ctrls[index]))
        {
            printk("ide%d:\n", index);

            irq_set_handler(ctrls[index].irq, ide_handler);

            if (have_drive(&ctrls[index], MASTER_DISK))
            {
                init_disk_reg(&ctrls[index],\
                               index,\
                               MASTER_DISK);
            }

            if (have_drive(&ctrls[index], SLAVE_DISK))
            {
                init_disk_reg(&ctrls[index],\
                               index,\
                               SLAVE_DISK);
            }
        }
    }

    return 0;
}

unsigned int read_sectors(struct ide_io_port *ctrl,\
                          sector_t            first,\
                          __u32               count,\
                          __u8               *buffer)
{
    /*Seteamos la cantidad de sectores a ser leidos.*/
    SET_COUNT_REG((*ctrl),count);
    SELECT_DRIVER((*ctrl),MASTER_DISK);
    /*Establecemos la direccion LBA28 del primer sector.*/
    SET_LBA28_ADDR((*ctrl),first);
    outb(0x1f1, (unsigned char)0x00);

    do_command(ctrl, ATA_READ, 1);

    {
        unsigned int end = (jiffies + HZ/2);

        while (jiffies < end);
    }

    copy_from_io(ctrl, buffer, count);

    return count;
}

void request(struct request_queue *queue)
{
    struct request     *rqst = queue->request_list;
    struct bio         *rbio = rqst->bio;
    struct ide_io_port *ctrl = rbio->bi_bdev->disk->private_data;
    
    /*Si la controladora esta ocupada (busy)
      volvemos a llamar al scheduler y continuamos
      mas adelante.*/
    if (CHECK_BIT_WIRQ((*ctrl), STS_BUSY))
        return;
        
    
}

void init_disks()
{
    /*Detectamos todos los discos en el sistema para
      luego registrarlos.*/

    
    wakeup_disks();

    /*unsigned char data[512];
    unsigned int  i = 0,j = 0;
    struct bio mb;
    struct bio_vec mbv;

    mbv.data = data;
    mb.bi_io_vec = &mbv;
    
    operations[0].operation = &mb;

    read_sectors(&(ctrls[0]), 65, 1, data);
    
    
    for (; i < 16; i++)
    {
        for (j = 0; j < 32; j++)
        {
            printk("%.2x", data[(i*32+j)]);
        }
        printk("\n");
    }*/
}











