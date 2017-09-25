#ifndef __FILE_WRITER_H__
#define __FILE_WRITER_H__

#include "config.h"
#include "log_buffer.h"

#define C_MAX_FILENAME_LEN        256
#define C_MAX_BUFFER_CNT          2

typedef struct {
    char            dir[C_MAX_FILENAME_LEN];
    char            filename[C_MAX_FILENAME_LEN];
    char            currfile[C_MAX_FILENAME_LEN];
    unsigned long   pid;
    unsigned long   split_size;
    int             reserve_cnt;
    char          **reserve_filename;
    unsigned long   content_size;
    int             using_fidx;

    log_buffer_t   *buffers[C_MAX_BUFFER_CNT];
    int             using_bufidx;

    mys_assemble_t  assemble;
    int             fd;
    void           *mutex;
    int             persistent_log_level;
} mys_filewriter_t;

mys_filewriter_t *file_writer_initialize(filelog_config_t *config, int file_mask);
int               write_log_file(mys_filewriter_t *file_writer, char *buf, int len, int force_write);
void              file_writer_finalize(mys_filewriter_t *file_writer);

#endif

