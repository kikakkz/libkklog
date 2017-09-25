#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "utils.h"
#include "log_tag.h"

#include "log_assemble.h"

int log_assemble(char *log_str, mys_logtag_t *tag, char *level_str,
        const char *tags, char *func, int line, char *thread, char *info)
{
    int len = 0;
    int i = 0;

    memset(tag, 0x0, sizeof(mys_logtag_t));

    log_tag_parse(tags, tag);
    len = log_timestamp(log_str);
    len += sprintf(log_str + len, "(%s)%s ", thread, level_str);
    for (i = 0; i < tag->count; i++) {
        len += sprintf(log_str + len, "<%s>", tag->tags[i]);
    }
    len += sprintf(log_str + len, "%s:%d: ", func, line);
    len += sprintf(log_str + len, "%s", info);
    return len;
}
