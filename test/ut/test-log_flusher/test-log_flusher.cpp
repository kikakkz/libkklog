#include "h2unit.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
extern "C" {
#include "../../../../libmyslog/src/flusher/log_flusher.c"
#include "../../../../libmyslog/src/observer/file_writer.c"
#include "../../../../libmyslog/src/observer/console_writer.c"
#include "../../../../libmyslog/src/observer/log_observer.c"
}
H2UNIT(log_flusher)
{
    void setup() {
    }
    void teardown() {
    }
};
static void fake_observer_outputs(void *observer, int tag_mask, char *buff, int len, int force_flusher )
{
    printf("this is a test log");
}

H2CASE(log_flusher, "log flusher")
{
    mys_logconfig_t config;
    memset(&config, 0x0, sizeof(mys_logconfig_t));
    config.force_flush_seconds = 5;
    config.force_flush_targets = 1;
    void  *observer = NULL;
    H2STUB(log_observer_output, fake_observer_outputs);

    log_flusher_initialize(&config, observer);

    H2EQ_MATH(1, g_flusher_running);
    H2EQ_MATH(1, config.force_flush_targets);

    log_flusher_finalize();

};
