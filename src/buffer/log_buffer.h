#ifndef _LOG__BUFF__H_
#define _LOG__BUFF__H_


typedef struct {
    char        *buffer;
    int          len;
    int          size;
    int          need_flush;
    void        *mutex;
}log_buffer_t;

log_buffer_t *log_buffer_new(int buffer_size);
int           log_buffer_del(log_buffer_t *buff);

#endif

