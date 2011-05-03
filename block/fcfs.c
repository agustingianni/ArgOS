#include <block/request.h>

/*Aca se implementa uno de los schedulers de E/S
  mas simples que existen(FCFS, First Come, First
  Served) con el unico proposito de hacer funcionar
  la capa de dispositivos de bloque.*/

/*Vamos a usar un solo request por ahora, y declarado
  de esta forma*/
static struct request tmp;

int fcfs_scheduler(struct request_queue *q, struct bio *bio)
{
    /*Ahora vemos como hacemos, porque hay que allocar mem*/
    tmp.bio      = bio;
    bio->bi_next = tmp.bio;
    tmp.biotail  = bio;

    tmp.next    = NULL;
    tmp.elevator_private = NULL;

    return 0;
}
