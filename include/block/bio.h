#ifndef BIO_H_
#define BIO_H_

#include <types.h>

struct bio;

/*Esta funcion es la llamada por el bottom_half
  una vez terminada la operacion.
  Los parametros son el bio, el numero de bytes de
  la operacion y el error*/
typedef unsigned int (bio_end_io_t)(struct bio *, __u32, __u32);

struct page
{
    int a;
};

struct bio_vec {
        //struct page *bv_page;
        /*Esto deberia ser una referencia a una pagina*/
        __u8        *data;
        __u32        bv_len;
        __u32        bv_offset;
};

/*0 for read.*/
#define BIO_RW_BIT  0x01

struct bio {
	sector_t             bi_sector; /* device address in 512 byte
                                           sectors */
	struct bio          *bi_next;   /* request queue link */
	struct block_device *bi_bdev;
	__u32                bi_flags;  /* status, command, etc */
	__u32                bi_rw;     /* bottom bits READ/WRITE*/
                                        /* top bits priority*/

	__u16                bi_vcnt;   /* how many bio_vec's */
	__u16                bi_idx;    /* current index into bio_vec */

	__u32                bi_size;   /*Bytes que restan operar*/

	struct bio_vec      *bi_io_vec; /* the actual vec list */

	bio_end_io_t        *bi_end_io; /*Para la respuesta asincronica*/
};

#endif
