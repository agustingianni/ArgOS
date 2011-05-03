#ifndef REQUEST_H_
#define REQUEST_H_

#include <block/bio.h>

struct request_queue;

typedef __u32 (schedul_request_fn)(struct request_queue *q,\
                                   struct bio *bio);

struct request_queue
{
    struct request *request_list;

    /*Punteros a las funciones del I/O-Scheduler*/
    schedul_request_fn *schedul_request;
};

struct request {
    struct request      *next;
    struct bio          *bio;
    struct bio          *biotail;
    void                *elevator_private;
};

#endif
