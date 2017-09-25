#include "h2unit.h"

extern "C" {
#include "../../../../libmyslog/src/observer/file_writer.c"
#include "../../../../libmyslog/src/observer/log_observer.c"
#include "../../../../libmyslog/src/observer/console_writer.c"
}

H2UNIT(log_observer)
{
    void setup() {
    }
    void teardown() {
    }
};

static int fake_write_file(mys_logwriter_t *writer, char *buff, int len, int force)
{
    return 1;
}

static mys_logwriter_t g_file_writer;

static mys_logwriter_t *fake_writer_initialize(mys_logconfig_t *config)
{
    g_file_writer.write = fake_write_file;
    return &g_file_writer;
}

static mys_logwriter_t *fake_console_writer_initialize(void)
{
    g_file_writer.write = fake_write_file;
    return &g_file_writer;
};

H2CASE(log_observer, "observer initialize and finalize")
{
    int output_mask = 3;
    void *observer = NULL;
    mys_logconfig_t config;

    H2STUB(file_writer_initialize, fake_writer_initialize);
    H2STUB(file_writer_finalize, fake_writer_initialize);
    H2STUB(console_writer_finalize, fake_writer_initialize);
    H2STUB(console_writer_initialize, fake_writer_initialize);
    memset(&config, 0x0, sizeof(mys_logconfig_t));
    strcpy(config.filename, "filename");

    observer = log_observer_initialize(output_mask, &config);
    mys_logobserver_t *observ = (mys_logobserver_t *)observer;

    H2EQ_TRUE(NULL != observer);
    H2EQ_MATH(2, observ->observer_count);
    H2EQ_TRUE(NULL !=  observ->writers[0]);
    H2EQ_TRUE(NULL !=  observ->writers[1]);
    H2EQ_TRUE(NULL ==  observ->writers[2]);
    H2EQ_TRUE(NULL ==  observ->writers[3]);

    log_observer_finalize(observer);
};

H2CASE(log_observer, "log observer output")
{
    int   output_mask = 3;
    char  *buff = "this is a test log\n";
    int   len = 20;
    mys_logconfig_t config;
    void *observer = NULL;

    memset(&config, 0x0, sizeof(mys_logconfig_t));
    strcpy(config.filename, "filename");

    H2STUB(console_writer_initialize, fake_console_writer_initialize);
    H2STUB(file_writer_initialize, fake_writer_initialize);
    H2STUB(file_writer_finalize, fake_writer_initialize);
    observer = log_observer_initialize(output_mask, &config);
    log_observer_output(observer, output_mask, buff, len);
    mys_logobserver_t *observ = (mys_logobserver_t *)observer;
    H2EQ_MATH(2, observ->observer_count);
    log_observer_finalize(observer);
};
