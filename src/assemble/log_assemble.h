#ifndef __LOG_ASSEMBLE_H__
#define __LOG_ASSEMBLE_H__

int log_assemble(char *log_str, mys_logtag_t *tag, char *level_str,
        const char *tags, char *func, int line, char *thread, char *info);

#endif
