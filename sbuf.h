typedef struct {
    int *buf;       /* buffer array */
    int n;          /* max slots */
    int front;      /* buf[(front+1) % n] is first item */
    int rear;       /* buf[rear % n] is last item */
    sem_t mutex;    /* protect access to buf */
    sem_t slots;    /* counts slots */
    sem_t items;    /* counts items */
} sbuf_t;

void sbuf_init(sbuf_t *sp, int n);
void sbuf_deinit(sbuf_t *sp);
void sbuf_insert(sbuf_t *sp, int item);
int sbuf_remove(sbuf_t *sp);