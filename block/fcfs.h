#ifndef FCFS_H_
#define FCFS_H_

/*Se retorna 0 en caso de exito.*/

__u32 fcfs_scheduler(struct request_queue *, struct bio *);

#endif