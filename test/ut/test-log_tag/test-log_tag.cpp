#include "h2unit.h"
extern "C" {
#include "../../../../libmyslog/src/assemble/log_tag.c"
}
H2UNIT(log_tag)
{
    void setup() {
    }
    void teardown() {
    }
};

H2CASE(log_tag, "parse tag")
{
    mys_logtag_t tag;
    char *tags = "tag1,tag2,tag3,tag4";

    memset(&tag, 0x0, sizeof(mys_logtag_t));
    H2EQ_MATH(0 , log_tag_parse(tags, &tag));
    H2EQ_MATH(4 ,tag.count);
    H2EQ_STRCMP("tag1", tag.tags[0]);
    H2EQ_STRCMP("tag2", tag.tags[1]);
    H2EQ_STRCMP("tag3", tag.tags[2]);
    H2EQ_STRCMP("tag4", tag.tags[3]);
};
