#ifndef __LOG_FILE_WRITERS_H__
#define __LOG_FILE_WRITERS_H__

#include "config.h"
#include "log_observer.h"

mys_logwriter_t *file_writers_initialize(filelog_config_t *config);
void             file_writers_finalize(mys_logwriter_t *writer);

#endif
