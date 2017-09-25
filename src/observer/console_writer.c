#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdlib.h>

#include "console_writer.h"
#include "mys_log.h"
#include "log_observer.h"

static int __write_log_console(mys_logwriter_t  *writer, mys_logmask_t *output_mask,
        mys_logcontent_t *logs, int force_write)
{
    mys_logcontent_t *log = NULL;
    if (NULL == output_mask || NULL == logs) {
        return  -1;
    }

    log = &logs[N_ASS_WPREFIX];
    fprintf(stdout, "%s", log->content);

    return 0;
}

mys_logwriter_t *console_writer_initialize(void)
{

    mys_logwriter_t *writer = (mys_logwriter_t *)malloc(sizeof(mys_logwriter_t));

    if (NULL == writer) {
        return NULL;
    }

    writer->write = __write_log_console;

    return writer;
}

void console_writer_finalize(mys_logwriter_t *writer)
{
    free(writer);
}
