#ifndef __LOG_TAG___H__
#define __LOG_TAG___H__

#define C_MAX_TAG_COUNT                 5
#define C_MAX_TAG_LEN                   32

typedef struct {
    char    tags[C_MAX_TAG_COUNT][C_MAX_TAG_LEN];
    int     count;
} mys_logtag_t;

int log_tag_parse(const char *tags, mys_logtag_t *tag);

#endif
