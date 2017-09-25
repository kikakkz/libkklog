#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "log_observer.h"
#include "file_writers.h"
#include "console_writer.h"
#include "utils.h"

typedef struct {
    int                 observer_count;
    mys_logwriter_t    *writers[C_MAX_LOGWRITER_COUNT];
    int                 output_mask;
} mys_logobserver_t;

void *log_observer_initialize(int output_mask, mys_logconfig_t *config)
{
    mys_logobserver_t *observer = (mys_logobserver_t *)malloc(sizeof(mys_logobserver_t));

    if (NULL == observer) {
        return NULL;
    }

    memset(observer, 0x0, sizeof(mys_logobserver_t));

    observer->output_mask = output_mask;
    observer->observer_count = 0;

    if (C_LOG_CATLOG_OUTPUT & output_mask) {

    }
    if (C_LOG_FILE_OUTPUT & output_mask) {
        observer->writers[N_LOGOUTPUT_FILE] = file_writers_initialize(&config->fileconfig);
        observer->observer_count++;
    }
    if (C_LOG_CONSOLE_OUTPUT & output_mask) {
        observer->writers[N_LOGOUTPUT_CONSOLE] = console_writer_initialize();
        observer->observer_count++;
    }
    if (C_LOG_NET_OUTPUT & output_mask) {

    }

    return observer;
}

int log_observer_output(void *observer, mys_logmask_t *output_mask, mys_logcontent_t *logs, int force_write)
{
    int i = 0;
    mys_logobserver_t *observ = NULL;

    if (NULL == (observ = (mys_logobserver_t *)observer) || NULL == output_mask) {
        return -1;
    }

    for (i = 0; i < C_MAX_LOGWRITER_COUNT; i++) {
        if (0 != (output_mask->output_mask & (1 << i))) {
            if (NULL != observ->writers[i]) {
                observ->writers[i]->write(observ->writers[i], output_mask, logs, force_write);
            }
        }

    }

    return 0;
}

void log_observer_finalize(void *observer)
{
    mys_logobserver_t *observ = (mys_logobserver_t *)observer;
    int outputs_mask = observ->output_mask;

    if (C_LOG_CATLOG_OUTPUT & outputs_mask) {

    }
    if (C_LOG_FILE_OUTPUT & outputs_mask) {
        file_writers_finalize(observ->writers[N_LOGOUTPUT_FILE]);
    }
    if (C_LOG_CONSOLE_OUTPUT & outputs_mask) {

    }
    if (C_LOG_NET_OUTPUT & outputs_mask) {

    }
    free(observer);
}
