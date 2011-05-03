#ifndef IDE_H_
#define IDE_H_

#include <arch/irq.h>
#include <types.h>
#include <drivers/ide/ata.h>
#include <block/request.h>

#include "../include/kernel/printk.h"

#define SECTOR_SIZE 512

/*Bit que selecciona el driver a usar por
  el controlador.
*/
#define MASTER_DISK  0x00
#define SLAVE_DISK   0x10

/*Los bits 7 y 5 son obsoletos (se usaban en MFM), y el
  bit 6 indica si se accede en modo LBA.*/
#define LBA_DRIVE_BITS_MASK 0xe0

#define MAGIC_NUMBER 0x88

/*
+----+----------------+---------------+
|Offt| Read (-IOR)    | Write (-IOW)  |
+----+----------------+---------------+-----------+
|    | ILLEGAL        | ILLEGAL       | <--+      |
|    | High Impedance | Not Used      | Control   |
|3XX | High Impedance | Not Used      | Block     |
|3XX | High Impedance | Not Used      | Registers |
|  6 | Altern Status  | Device Control|    |      |
|  7 | Drive Address  | Not Used      | <--+      |
+----+----------------+---------------+-----------+
|  0 | Data Port      | Data Port     | <--+      |
|  1 | Error Register | Precomp       |    |      |
|  2 | Sector Count   | Sector Count  | Command   |
|  3 | Sector Number  | Sector Number | Block     |
|  4 | Cylinder Low   | Cylinder Low  | Registers |
|  5 | Cylinder High  | Cylinder High |    |      |
|  6 | Drive / Head   | Drive / Head  |    |      |
|  7 | Status         | Command       | <--+      |
+----+----------------+---------------+-----------+
*/
/*Offsets para los registros de comando y control*/
/*Control*/
/*Lectura*/
#define CTR_ALT_STATUS_OFFT  0x00
#define CTR_DRV_ADDR_OFFT    0x01
/*Escritura*/
#define CTR_DEV_CTR_OFFT     0x00

/*Commandos*/
/*Lectura*/
#define CMD_ERROR_OFFT       0x01
#define CMD_STATUS_OFFT      0x07
/*Escritura*/
#define CMD_PRECOMP_OFFT     0x01
#define CMD_COMMAND_OFFT     0x07
/*Lectura-Escritura*/
#define CMD_DATA_OFFT        0x00
#define CMD_SECTOR_CNT_OFFT  0x02
#define CMD_SECTOR_NUM_OFFT  0x03
#define CMD_CYLIN_LOW_OFFT   0x04
#define CMD_CYLIN_HIGH_OFFT  0x05
#define CMD_DRIVE_HEAD_OFFT  0x06

/*Status Register Bits*/
/*Busy*/
#define STS_BUSY 0x80
/*Drive Ready*/
#define STS_DRDY 0x40
/*Drive Write Fault*/
#define STS_DWF  0x20
/*Drive Seek Complete*/
#define STS_DSC  0x10
/*Data Request*/
#define STS_DRQ  0x08
/*Corrected Data*/
#define STS_CORR 0x04
/*Index. Set when the index mark is detected
  once per disk revolution.*/
#define STS_INDX 0x02
/*Error*/
#define STS_ERR  0x01

/*The standard port, and irq values are these:

	ide0=0x1f0,0x3f6,14
	ide1=0x170,0x376,15
	ide2=0x1e8,0x3ee,11
	ide3=0x168,0x36e,10
*/

#define ATA_TYPE_TO_INDEX(type,index) \
(\
    ((type == ATA_ATAPI_TYPE_DIRECT)) ?(index=0):\
    ((type &  ATA_ATAPI_TYPE_TAPE)    ?(index=1):\
    ((type &  ATA_ATAPI_TYPE_CDROM)   ?(index=2):\
    ((type &  ATA_ATAPI_TYPE_OPTICAL) ?(index=3):(index=4))))\
)\

char * ata_types[] = {\
                        "DISK"   ,\
                        "TAPE"   ,\
                        "CDROM"  ,\
                        "OPTICAL",\
                     };

#define SECTOR_TO_MEGA(sects,megas) (megas = sects/2048)

/*Este par de macros devuelven el estado del bit 'bit' que
  le especifiquemos, diferenciandoce en que, la primera no
  borra la interrupcion pendiente, mientras que la segunda
  si.*/
#define CHECK_BIT_WIRQ(hdport,bit)  (inb(hdport.control +\
                                     CTR_ALT_STATUS_OFFT) & bit)
#define CHECK_BIT_WOIRQ(hdport,bit) (inb(hdport.command +\
                                     CMD_STATUS_OFFT) & bit)

/*Estas macros deshabilitan y habilitan la interrupcion
  generada por la controladora luego de la ejecucion de
  un comando.
  DCR_DEFAULT_BITS contiene los bits usados por defecto
  para para operar con este registro, colocando el bit
  de interrupcion en activo.
  Los otros 2 defines, marcan los bits de reset e
  interrupt enable(El cual esta activo cuando esta
  en 0), respectivamente.*/
#define DCR_DEFAULT_BITS 0x08
#define DCR_SRST         0x04
#define DCR_NIEN         0x02
#define IDE_IRQ_DISABLE(ctrl) (outb((ctrl.control + CTR_DEV_CTR_OFFT)\
                                   , (DCR_DEFAULT_BITS | DCR_NIEN)))

#define IDE_IRQ_ENABLE(ctrl)  (outb((ctrl.control + CTR_DEV_CTR_OFFT)\
                                   ,  DCR_DEFAULT_BITS))

/*La macro 'SET_HEAD_BITS' existe para setear los bits 'bits' en la parte
  del registro que corresponde a los cabezales. Con 'SELECT_DRIVER' es
  similar, establece el driver 'driver' en el registro.
  Todos los cambios hechos con estas 2 macros, no se veran reflejados
  sobre los registros del controlador, asta que se llame a la macro
  'FLUSH_HEDR_REG' la cual hara que estos cambios sean validos.*/
#define SET_HEAD_BITS(ctrl,bits)   (operations[ctrl.index].driver =\
                                  ((operations[ctrl.index].driver & 0xf0) | bits))

#define SELECT_DRIVER(ctrl,drv) (operations[ctrl.index].driver =\
                     ((operations[ctrl.index].driver & (LBA_DRIVE_BITS_MASK | 0x0f)) | drv))

#define FLUSH_HEDR_REG(ctrl) (outb((ctrl.command + CMD_DRIVE_HEAD_OFFT),\
                                    operations[ctrl.index].driver))


#define SET_COUNT_REG(ctrl,count) (outb((ctrl.command + CMD_SECTOR_CNT_OFFT), count))
/*Con esta macro seteamos la direccion LBA28 'addr' en el
  controlador 'ctrl'*/
#define SET_LBA28_ADDR(ctrl,addr) \
{\
    outb((ctrl.command + CMD_SECTOR_NUM_OFFT), ((unsigned char *)&addr)[0]);\
    outb((ctrl.command + CMD_CYLIN_LOW_OFFT ), ((unsigned char *)&addr)[1]);\
    outb((ctrl.command + CMD_CYLIN_HIGH_OFFT), ((unsigned char *)&addr)[2]);\
    SET_HEAD_BITS(ctrl,((unsigned char *)&addr)[3]);\
    FLUSH_HEDR_REG(ctrl);\
}

struct ide_io_port
{
    /*Registro de comandos, control, irq
      y driver seleccionado*/
    __u16 command;
    __u16 control;
    __u8  irq;
    /*Colocar el numero de indice aca
      acelera algunas cosas.*/
    __u8  index;
};

/*En el campo driver se guarda no solo la parte
  de seleccion del driver, sino tambien los 4 bits
  de head.
  En 'operation' se guarda el ultimo comando ejecutado
  y en caso de no haber nada en proceso, apunta a NULL*/
struct ctrl_operation
{
    struct bio *operation;
    __u8 driver;
};

/*Devuelve informacion sobre el dispositivos
  conectado en la unidad que se le especifique
  por medio del 2do y 3er parametro
  (el 3er parametro es la macro
  MASTER_DISK o SLAVE_DISK).*/
void get_ata_info(struct ata_params  *,\
                  struct ide_io_port *,\
                  unsigned char);

/*Realiza la lectura de count sectores
  desde el dispositivo indicado por
  dev y se almacena en buffer*/
__u8 copy_from_io(struct ide_io_port *dev,\
                  unsigned char      *buffer,\
                  unsigned int        count);

/*'bottom_half' es la funcion que continua el trabajo
   del manejador de interrupciones, con la intencion
   de que sea scheduleada para competir por los recursos.*/
void bottom_half(void);

/*Este es el manejador de interrupciones
  que sera llamado por el disco cuando
  la operacion que se puso en marcha
  finalice.*/
void ide_handler(icontext_t *);

/*Comprueba la existencia de un controlador.
  Devuelve != 0 en caso de que exista */
__u32 controler_exist(struct ide_io_port *);

/*Evalua la existencia o no del driver que
  le especifiquemos.*/
__u32 have_drive(struct ide_io_port *, __u8);

void init_disk_reg(struct ide_io_port *, __u32, __u8);

__u32  wakeup_disks(void);

void request(struct request_queue *queue);

/*Pone en marcha todas las operaciones
  necesarias para la deteccion y
  posterior registro de los discos.*/
void init_disks();

/*Con esta funcion, buscamos a que indice del
  arreglo 'ctrls', pertenece la irq pasada, para
  usar las direcciones dentro del handler.*/
__inline__ __u32 get_index_from_irq(__u8 irq)
{
    /*Las irq estan desplazadas 32 unidades*/
    irq -= 32;
    if (irq >= 14)
        return (irq - 14);
    else
        return (13 - irq);
}

/*Habiendo configurado el controlador 'ctrl', con
  'do_command' ponemos en marcha el comando
  'command', pudiendo hacer que se genere una
  interrupcion al final('irq' distinto de 0),
  o simplemente que vuelva cuando el comando
  alla concluido.
*/
__inline__ void do_command(struct ide_io_port *ctrl,\
                           __u8 command,\
                           __u8 irq)
{
    

    //IDE_IRQ_DISABLE((*ctrl));

    /*Esperamos que el disco este listo para
      aceptar comandos.*/
    while (!CHECK_BIT_WIRQ((*ctrl), STS_DRDY));

    if (irq)
        IDE_IRQ_ENABLE((*ctrl));

    /*Escribirmos el comando en el registros
      correspondiente del controlador.*/
    outb((ctrl->command + CMD_COMMAND_OFFT), command);

    /*En caso de no querer que se use el manejador
      de interrupciones, procedemos como sigue.*/
    if (!irq)
        while(CHECK_BIT_WOIRQ((*ctrl), STS_BUSY));
}

#endif
