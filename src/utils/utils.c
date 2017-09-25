#include <stdio.h>

#include "utils.h"

#include "pltf_time.h"

int log_timestamp(char *timestamp)
{
    char now_str[250];
    int len = 250;

    pltf_time_now_tostring(1, now_str, &len);
    return sprintf(timestamp, "[%s]", now_str);
}
