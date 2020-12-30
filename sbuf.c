#include "csapp.h"

void sbuf_init(sbuf_t *sp, int n)
{
    sp->buf = Calloc(n, sizeof(int));
    sb->n = n;
    sp->front = sp->rear = 0;   /* no front/rear when init */
    Sem_init(&sp->mutex, 0, 1); /* buf lock */
    Sem_init(&sp->slots, 0, n); /* n slots when init */
    Sem_init(&sp->items, 0, 0); /* 0 items when init */
}

void sbuf_deinit(sbuf_t *sp)
{
    Free(sp->buf);
}

void sbuf_insert(sbuf_t *sp, int item)
{
    P(&p->slots);   /* wait for available slot */
    P(&p->mutex);   /* lock buffer until item is inserted */
    sp->buf[(++sp->rear) % (sp->n)] = item; /* insert item */
    V(&p->mutex);
    V(&p->items);   /* announce that new item is available */
}

int sbuf_remove(sbuf_t *sp)
{
    int item;
    P(&p->items);   /* wait for available item */
    P(&p->mutex);   /* lock buffer until item is removed */
    item = [(++sp->front) % (sp->n)];   /* remove item */
    V(&p->mutex);
    V(&p->slots);   /* announce new available item */
    return item;
}