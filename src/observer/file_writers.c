#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "file_writer.h"
#include "log_observer.h"
#include "file_writers.h"

typedef struct {
    mys_filewriter_t  *writers[C_MAX_FILELOG_CNT];
    int count;
} mys_filewriters_t;

static int __write_log_files(mys_logwriter_t *writer, mys_logmask_t *output_mask,
        mys_logcontent_t *logs, int force_writer)
{
    int i = 0, file_mask = 0;
    int len = 0;
    char *logstr = NULL;
    mys_logcontent_t *log = NULL;

    mys_filewriters_t *filewriters = NULL ;
    if (NULL == (filewriters =
                (mys_filewriters_t *)writer->priv)) {
        return -1;
    }
    if (NULL == output_mask) {
        return -1;
    }

    file_mask = output_mask->fd_mask[N_LOGOUTPUT_FILE];

    for (i = 0; i < filewriters->count; i++) {
        if (0 != (file_mask & (1 << i))) {
            if (logs) {
                log = &logs[filewriters->writers[i]->assemble];
                logstr = log->content;
                len = log->length;
            } else {
                logstr = NULL;
                len = 0;
            }
            write_log_file(filewriters->writers[i], logstr, len, force_writer);
        }
    }

    return 0;
}

mys_logwriter_t *file_writers_initialize(filelog_config_t *config)
{
    int i = 0;
    mys_logwriter_t *writer = NULL;
    mys_filewriters_t *filewriters = NULL;

    if (NULL == (writer = (mys_logwriter_t *)malloc(
                    sizeof(mys_logwriter_t)))) {
        goto L_ERROR_LOGWRITER;
    }

    if (NULL == (filewriters =
                (mys_filewriters_t *)malloc(sizeof(mys_filewriters_t)))) {
        goto L_ERROR_FILEWRITERS;
    }

    for (i = 0; i < config->file_cnt; i++) {
        if (NULL == (filewriters->writers[i] =
                    file_writer_initialize(config, i))) {
            goto L_ERROR_INIT;
        }
    }

    if (C_MAX_FILELOG_CNT < config->file_cnt) {
        filewriters->count = C_MAX_FILELOG_CNT;
    } else {
        filewriters->count = config->file_cnt;
    }

    writer->write = __write_log_files;
    writer->priv = filewriters;

    return writer;

L_ERROR_INIT:
    for (i = 0; i < filewriters->count; i++) {
        if (filewriters->writers[i]) {
            file_writer_finalize(filewriters->writers[i]);
        }
    }
L_ERROR_FILEWRITERS:
    free(writer);
L_ERROR_LOGWRITER:
    return NULL;
}

void file_writers_finalize(mys_logwriter_t *writer)
{
    int i = 0;
    if (NULL == writer) {
        return;
    }
    mys_filewriters_t *filewriters = (mys_filewriters_t *)writer->priv;
    for (i = 0; i < filewriters->count; i++) {
        if (filewriters->writers[i]) {
            file_writer_finalize(filewriters->writers[i]);
            free(filewriters->writers[i]);
        }
    }
    free(filewriters);
    free(writer);
}
