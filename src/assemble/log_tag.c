#include "log_tag.h"

#include <string.h>
#include <stdlib.h>

#include "pltf_string.h"

int log_tag_parse(const char *tags, mys_logtag_t *tag)
{
    char *token = NULL;
    char *ltags = NULL;
    char *savedptr = NULL;

    if (NULL == tags || NULL == tag || 0 == strlen(tags)) {
        return -1;
    }

    memset(tag, 0x0, sizeof(mys_logtag_t));

    if (NULL == (ltags = (char *)malloc(
                    strlen(tags) + 1))) {
        return -1;
    }

    memset(ltags, 0x0, strlen(tags) + 1);
    strcpy(ltags, tags);

    token = pltf_strtok(ltags, ",", &savedptr);

    while (NULL != token) {
        strcpy(tag->tags[tag->count++], token);

        if (C_MAX_TAG_COUNT <= tag->count) {
            break;
        }
        token = pltf_strtok(NULL, ",", &savedptr);
    }

    free(ltags);
    return 0;
}
