#include "h2unit.h"
extern "C" {
#include "../../../../libmyslog/src/observer/console_writer.c"
}
H2UNIT(console_writer)
{
    void setup() {
    }
    void teardown() {
    }
};

H2CASE(console_writer, "console writer initialize")
{
};
