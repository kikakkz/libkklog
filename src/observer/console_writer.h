#ifndef __LOG_CONSOLE_WRITER_H__
#define __LOG_CONSOLE_WRITER_H__


#include "log_observer.h"

mys_logwriter_t *console_writer_initialize(void);
void             console_writer_finalize(mys_logwriter_t *writer);

#endif
