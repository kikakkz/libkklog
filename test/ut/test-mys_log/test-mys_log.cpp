#include "h2unit.h"
extern "C" {
#include "../../../../libmyslog/src/mys_log.c"
#include "../../../../libmyslog/src/observer/log_observer.c"
#include "../../../../libmyslog/src/observer/file_writer.c"
#include "../../../../libmyslog/src/observer/console_writer.c"
#include "../../../../libmyslog/src/assemble/log_assemble.c"
#include "../../../../libmyslog/src/assemble/log_tag.c"
#include "../../../../libmyslog/src/config/config.c"
#include "../../../../libmyslog/src/utils/utils.c"
#include "../../../../libmyslog/src/utils/clock.c"

}
H2UNIT(mys_log)
{
    void setup() {
    }
    void teardown() {
    }
};

H2CASE(mys_log, "level to string")
{
    H2EQ_STRCMP("N/A", __level_tostring((mys_loglevel_t)123));
    H2EQ_STRCMP("N", __level_tostring(N_LOG_NONE));
    H2EQ_STRCMP("E", __level_tostring(N_LOG_ERROR));
    H2EQ_STRCMP("W", __level_tostring(N_LOG_WARNING));
    H2EQ_STRCMP("A", __level_tostring(N_LOG_ACCESS));
    H2EQ_STRCMP("I", __level_tostring(N_LOG_INFO));
    H2EQ_STRCMP("D", __level_tostring(N_LOG_DEBUG));
    H2EQ_STRCMP("T", __level_tostring(N_LOG_TRACE));
};

static int fake_get_outputs(mys_logconfig_t *config)
{
    return  3;
}

static mys_logconfig_t *fake_log_config_initialize(char *filename)
{
    static mys_logconfig_t config;
    strcpy(config.filename, "filename");
    return &config;
}

H2CASE(mys_log, "log initialize")
{
    char filename[] = "./myslog.conf";

    H2STUB(log_config_get_outputs, fake_get_outputs);
    H2STUB(log_config_initialize, fake_log_config_initialize);
    H2EQ_MATH(0, mys_log_initialize(filename));
    H2EQ_MATH(1, g_log_desc.initialized);
    mys_log_finalize();
};

H2CASE(mys_log, " get thread name")
{
    char name[] = "name";
    char filename[] = "./myslog.conf";

    H2STUB(log_config_get_outputs, fake_get_outputs);
    H2STUB(log_config_initialize, fake_log_config_initialize);
    mys_log_initialize(filename);
    mys_log_set_thread(name);
    H2EQ_STRCMP(name, __get_thread_name());
    mys_log_finalize();
};

H2CASE(mys_log, "log write")
{
};
