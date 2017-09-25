#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "pltf_thread.h"
#include "log_buffer.h"

log_buffer_t *log_buffer_new(int buffer_size)
{
    log_buffer_t *buff = NULL;

    if (buffer_size <= 0) {
        goto L_ERROR;
    }

    if (NULL == (buff =
                malloc(sizeof(log_buffer_t)))) {
        goto L_ERROR;
    }

    memset(buff, 0, sizeof(log_buffer_t));
    if (NULL == (buff->buffer =
                (char *)malloc(buffer_size))) {
        goto L_ERROR_BUFFER_ALLOC;
    }

    if (NULL == (buff->mutex =
                pltf_mutex_init())) {
        goto L_ERROR_MUTEX_INIT;
    }

    memset(buff->buffer, 0, buffer_size);
    buff->size = buffer_size;
    buff->len = 0;
    buff->need_flush = 0;

    return buff;

L_ERROR_MUTEX_INIT:
    free(buff->buffer);
L_ERROR_BUFFER_ALLOC:
    free(buff);
L_ERROR:
    return NULL;
}

int log_buffer_del(log_buffer_t *buff)
{
    if (NULL == buff ) {
        return -1;
    }

    pltf_mutex_destroy(buff->mutex);
    free(buff->buffer);
    free(buff);

    return 0;
}
