#ifndef __LOG_CLOCK_H__
#define __LOG_CLOCK_H__

#include "config.h"

#define C_FLUSH_TIMEOUT     1
#define C_FLUSH_FULL        2

void log_flusher_initialize(mys_logconfig_t *config, void *observer);
void log_flusher_finalize(void);
int  log_flusher_signal(void);
#endif
