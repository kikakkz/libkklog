#ifndef __LOG_OBSERVER___H___
#define __LOG_OBSERVER___H___

#include "config.h"
#include "mys_log.h"

#define C_MAX_BUFFER_LEN          (1024 * 1024)

typedef struct _mys_logwriter_t {
    void    *priv;
    int     (*write)(struct _mys_logwriter_t *writer, mys_logmask_t *output_mask,
                     mys_logcontent_t *logs, int force_write);
} mys_logwriter_t;


void  *log_observer_initialize(int output_mask, mys_logconfig_t *config);
int    log_observer_output(void *observer, mys_logmask_t *output_mask,
                    mys_logcontent_t *logs, int force_write);
void   log_observer_finalize(void *observer);

#endif
