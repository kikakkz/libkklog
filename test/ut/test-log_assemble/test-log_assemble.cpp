#include "h2unit.h"

extern "C" {
#include "../../../../libmyslog/src/utils/utils.c"
#include "../../../../libmyslog/src/assemble/log_assemble.c"
#include "../../../../libmyslog/src/assemble/log_tag.c"
}

H2UNIT(log_assemble)
{
    void setup() {
    }
    void teardown() {
    }
};

static void fake_pltf_time_now_tostring(int cache, char *str, int *len)
{
    sprintf(str, "1234:12:12");
}
H2CASE(log_assemble, "assemble log")
{
    H2STUB(pltf_time_now_tostring, fake_pltf_time_now_tostring);
    mys_logtag_t tag;
    char *tags = "tag1,tag2,tag3,tag4";
    char log_str[2048] = {0} ;
    char *level_str = "N";
    char *func = "function";
    int line = 2;
    char *thread = "thread";
    char *info = "this is thread 1 log, number = 1";
    char *output = "[1234:12:12](thread)N <tag1><tag2><tag3><tag4>function:2: this is thread 1 log, number = 1";

    memset(&tag, 0x0, sizeof(mys_logtag_t));
    log_assemble(log_str, &tag, level_str, tags, func, line, thread, info);
    H2EQ_STRCMP(output, log_str);
};
