#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "pltf_thread.h"
#include "pltf_file.h"
#include "pltf_time.h"
#include "pltf_process.h"

#include "file_writer.h"
#include "log_observer.h"
#include "config.h"
#include "mys_log.h"
#include "log_flusher.h"

#define C_DEFAULT_SPLIT_SIZE      1024 * 1024 * 1024
#define C_DEFAULT_RESERVE_CNT     10

static int __assemble_new_filename(mys_filewriter_t *file_writer, char *filename)
{
    int len = 0;
    if (NULL == filename || NULL == file_writer) {
        return -1;
    }
    len = sprintf(filename, "%s", file_writer->dir);
    len += sprintf(filename + len, "/%s", file_writer->filename);
    len += sprintf(filename + len, "_%lu", file_writer->pid);
    len += sprintf(filename + len, "_%lu", pltf_time_now(0));

    return 0;
}

static int __recreate_log_file(mys_filewriter_t *file_writer)
{
    char filename[C_MAX_FILENAME_LEN] = {0};
    int index = file_writer->using_fidx;

    if (file_writer->fd < 0 ||
            pltf_file_close(file_writer->fd) < 0) {
    }

    __assemble_new_filename(file_writer, filename);
    pltf_file_rename(file_writer->currfile, filename);

    if ((file_writer->fd = pltf_file_open(
                    file_writer->currfile,
                    N_FILEMODE_ASCII,
                    N_FILEFLAG_TRUNC)) < 0) {
        return -1;
    }

    index = (index + 1) % file_writer->reserve_cnt;
    file_writer->using_fidx = index;

    if (NULL != file_writer->reserve_filename[index] &&
            0 < strlen(file_writer->reserve_filename[index])) {
        pltf_file_del(file_writer->reserve_filename[index]);
    }

    strcpy(file_writer->reserve_filename[index], filename);

    return 0;
}

static int __splitable(mys_filewriter_t *file_writer)
{
    return (file_writer->content_size > file_writer->split_size) ? 1 : 0;
}

static int __write_file(mys_filewriter_t *file_writer, log_buffer_t *buffer)
{
    if (NULL == file_writer || NULL == buffer) {
        return -1;
    }

    pltf_mutex_lock(buffer->mutex);

    if (0 <= pltf_file_write(file_writer->fd,
                    buffer->buffer, buffer->len)) {
        file_writer->content_size += buffer->len;
    }

    buffer->len = 0;
    buffer->need_flush = 0;

    pltf_mutex_unlock(buffer->mutex);

    return 0;
}

#define __BUFFER_FLUSHABLE(buffer, force_write)     \
    ((force_write == C_FLUSH_TIMEOUT ||             \
      buffer->need_flush) &&                        \
     (0 < buffer->len))

static int __flush_buffer(mys_filewriter_t *file_writer, int force_write)
{
    int i = 0;
    log_buffer_t *buffer = NULL;
    int index = 0;

    pltf_mutex_lock(file_writer->mutex);
    index = file_writer->using_bufidx;
    pltf_mutex_unlock(file_writer->mutex);

    for (i = index + 1; i < C_MAX_BUFFER_CNT; i++) {
        buffer = file_writer->buffers[i];
        if (__BUFFER_FLUSHABLE(buffer, force_write)) {
            __write_file(file_writer, buffer);
        }
    }

    for (i = 0; i <= index; i++) {
        buffer = file_writer->buffers[i];
        if (__BUFFER_FLUSHABLE(buffer, force_write)) {
            __write_file(file_writer, buffer);
        }
    }

    return 0;
}

static void __append_buffer(mys_filewriter_t *file_writer, char *buf, int len)
{
    int index = 0;
    log_buffer_t *buffer = NULL;
    int retries = 3;

    pltf_mutex_lock(file_writer->mutex);
    index = file_writer->using_bufidx;
    pltf_mutex_unlock(file_writer->mutex);

    buffer = file_writer->buffers[index];

BUFFER_FLUSH:
    pltf_mutex_lock(buffer->mutex);
    if (buffer->size < buffer->len + len) {
        buffer->need_flush = 1;
        index = (index + 1) % C_MAX_BUFFER_CNT;

        pltf_mutex_lock(file_writer->mutex);
        file_writer->using_bufidx = index;
        pltf_mutex_unlock(file_writer->mutex);

        log_flusher_signal();
    }
    pltf_mutex_unlock(buffer->mutex);

    buffer = file_writer->buffers[index];
    pltf_mutex_lock(buffer->mutex);
    if (buffer->len + len <= buffer->size) {
        memcpy(&buffer->buffer[buffer->len], buf, len);
        buffer->len += len;
    } else {
        if (0 < retries--) {
            pltf_mutex_unlock(buffer->mutex);
            goto BUFFER_FLUSH;
        }
    }
    pltf_mutex_unlock(buffer->mutex);
}

int write_log_file(mys_filewriter_t *file_writer,
        char *buf, int len, int force_write)
{
    if (force_write) {
        __flush_buffer(file_writer, force_write);
    }

    if (NULL != buf && 0 != len) {
        __append_buffer(file_writer, buf, len);
    }

    if (__splitable(file_writer)) {
        pltf_mutex_lock(file_writer->mutex);
        if (__splitable(file_writer)) {
            __recreate_log_file(file_writer);
            file_writer->content_size = 0;
        }
        pltf_mutex_unlock(file_writer->mutex);
    }

    return 0;
}



static int  __malloc_reserve_filename(mys_filewriter_t *file_writer)
{
    int i;

    if (NULL == (file_writer->reserve_filename =
                (char **)malloc(file_writer->reserve_cnt * sizeof(char *)))) {
        goto L_ERROR;
    }

    for (i = 0; i < file_writer->reserve_cnt; i++) {
        if (NULL == (file_writer->reserve_filename[i] =
                    (char *)malloc(C_MAX_FILENAME_LEN * sizeof(char)))) {
            goto L_ERROR_FILENAME;
        }
        memset(file_writer->reserve_filename[i], 0, C_MAX_FILENAME_LEN * sizeof(char));
    }

    return 0;

L_ERROR_FILENAME:
    for (i = 0; i < file_writer->reserve_cnt; i++) {
        if (file_writer->reserve_filename[i]) {
            free(file_writer->reserve_filename);
        }
    }
    free(file_writer->reserve_filename);
L_ERROR:
    return -1;
}
static int __prepare_file_writer(mys_filewriter_t *file_writer,
         filelog_config_t *config, int file_mask)
{
    int buffer_size = 0, i = 0;

    sprintf(file_writer->dir, "%s",
            config->files[file_mask].dir);
    sprintf(file_writer->filename, "%s",
            config->files[file_mask].filename);

    if (strlen(file_writer->dir) + strlen(file_writer->filename)
            > C_MAX_FILENAME_LEN) {
        goto L_ERROR;
    }

    sprintf(file_writer->currfile, "%s/%s",
            file_writer->dir, file_writer->filename);

    if ((file_writer->fd = pltf_file_open(
                    file_writer->currfile,
                    N_FILEMODE_ASCII,
                    N_FILEFLAG_TRUNC)) < 0) {
        goto L_ERROR;
    }

    if (NULL == (file_writer->mutex = pltf_mutex_init())) {
        goto L_ERROR_MUTEX;
    }

    if (0 == config->buffer_size) {
        buffer_size = C_MAX_BUFFER_LEN;
    } else {
        buffer_size = config->buffer_size;
    }

    for (i = 0; i < C_MAX_BUFFER_CNT; i++) {
        if (NULL == (file_writer->buffers[i] =
                    log_buffer_new(buffer_size))) {
            goto L_ERROR_BUFFER;
        }
    }


    if (0 >= config->files[file_mask].reserve_cnt) {
        file_writer->reserve_cnt = C_DEFAULT_RESERVE_CNT;
    } else {
        file_writer->reserve_cnt = config->files[file_mask].reserve_cnt;
    }

    if (__malloc_reserve_filename(file_writer) < 0) {
        goto L_ERROR_RESERVE;
    }

    if (0 >= config->files[file_mask].split_size) {
        file_writer->split_size = C_DEFAULT_SPLIT_SIZE;
    } else {
        file_writer->split_size = config->files[file_mask].split_size;
    }

    file_writer->pid = pltf_get_pid();
    file_writer->content_size = 0;
    file_writer->using_bufidx = 0;
    file_writer->using_fidx = 0;
    file_writer->assemble = config->files[file_mask].assemble;

    strcpy(file_writer->reserve_filename[0], file_writer->currfile);

    return 0;

L_ERROR_RESERVE:
L_ERROR_BUFFER:
    for (i = 0; i < C_MAX_BUFFER_CNT; i++) {
        if (file_writer->buffers[i]) {
           log_buffer_del(file_writer->buffers[i]);
        }
    }
    pltf_mutex_destroy(file_writer->mutex);
L_ERROR_MUTEX:
    pltf_file_close(file_writer->fd);
L_ERROR:
    return -1;
}

static void __free_reserve_filename(mys_filewriter_t *file_writer)
{
    int i;

    for (i = 0; i < file_writer->reserve_cnt; i++) {
        free(file_writer->reserve_filename[i]);
    }

    free(file_writer->reserve_filename);
}

mys_filewriter_t *file_writer_initialize(filelog_config_t *config, int file_mask)
{

    mys_filewriter_t *file_writer = NULL;

    if (NULL == (file_writer = (mys_filewriter_t *)malloc(
                    sizeof(mys_filewriter_t)))) {
        goto L_ERROR_MALLOC_FILE_WRITER;
    }

    if (__prepare_file_writer(file_writer, config, file_mask) < 0) {
        goto L_ERROR_PREPARE_FILE_WRITER;
    }

    return file_writer;

L_ERROR_PREPARE_FILE_WRITER:
    free(file_writer);
L_ERROR_MALLOC_FILE_WRITER:
    return NULL;
}

void file_writer_finalize(mys_filewriter_t *file_writer)
{
    int i;

    __flush_buffer(file_writer, C_FLUSH_TIMEOUT);

    pltf_mutex_destroy(file_writer->mutex);
    for (i = 0; i < C_MAX_BUFFER_CNT; i++) {
        log_buffer_del(file_writer->buffers[i]);
    }
    pltf_file_close(file_writer->fd);

    __free_reserve_filename(file_writer);
}
