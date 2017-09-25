#include <unistd.h>
#include "h2unit.h"
#include <pthread.h>

extern "C" {
#include "../../../../libmyslog/src/observer/file_writer.c"
}
H2UNIT(file_writer)
{
    void setup() {
    }
    void teardown() {
    }
};

#define C_MAX_FAKEFILE_LEN        (1024 * 1024)

static char g_fake_file[C_MAX_FAKEFILE_LEN] = {0};

static int fake_pltf_write_file(int fd, char *buffer, int len)
{
    memcpy(g_fake_file, buffer, len);
    return 0;
}

static int fake_write_file(mys_filewriter_t *filewriter, log_buffer_t *buffer)
{
    buffer->len = 0;
    buffer->need_flush = 0;
    return 0;
}

static void fake_flusher_signal()
{

}

static int fake_append_buffer(mys_logwriter_t  *file_writer, char *buf, int len)
{
    return 0;
}

static int fake_flush_buffer(mys_logwriter_t *file_writer)
{
    return 0;
}

static void fake_log_buffer_del()
{

}

H2CASE(file_writer, "write file")
{
    H2STUB(pltf_file_write, fake_pltf_write_file);

    mys_filewriter_t filewriter;
    log_buffer_t buffer;
    char *buf = "this is write file test";

    memset(&filewriter, 0, sizeof(mys_filewriter_t));
    memset(&buffer, 0, sizeof(log_buffer_t));

    buffer.buffer = buf;
    buffer.len = strlen(buf) + 1;
    buffer.need_flush = 1;

    H2EQ_MATH(0, __write_file(&filewriter, &buffer));
    H2EQ_MATH(0, buffer.need_flush);
    H2EQ_TRUE(strstr(g_fake_file, buf));
}

H2CASE(file_writer, "flush buffer")
{
    H2STUB(__write_file, fake_write_file);

    mys_filewriter_t file_writer;
    log_buffer_t buffer1, buffer2;
    char *notfull = "This is not full\n";
    char *full = "this is a full buffer\n";

    memset(&file_writer, 0x0, sizeof(mys_filewriter_t));
    memset(&buffer1, 0x0, sizeof(log_buffer_t));
    memset(&buffer2, 0x0, sizeof(log_buffer_t));

    buffer1.buffer = notfull;
    buffer1.need_flush = 0;
    buffer1.len = strlen(notfull) + 1;

    buffer2.buffer = full;
    buffer2.need_flush = 1;
    buffer2.len = strlen(full) + 1;

    file_writer.buffers[0] = &buffer1;
    file_writer.buffers[1] = &buffer2;
    file_writer.using_bufidx = 0;

    H2EQ_TRUE(0 == __flush_buffer(&file_writer, C_FLUSH_FULL));
    H2EQ_MATH(0, buffer2.len);
    H2EQ_MATH(0, buffer2.need_flush);
    H2EQ_MATH(strlen(notfull) + 1, buffer1.len);
    H2EQ_STRCMP(notfull, buffer1.buffer);
    
    H2EQ_TRUE(0 == __flush_buffer(&file_writer, C_FLUSH_TIMEOUT));
    H2EQ_MATH(0, buffer2.len);
    H2EQ_MATH(0, buffer2.need_flush);
    H2EQ_MATH(0, buffer1.len);
    H2EQ_MATH(0, buffer1.need_flush);
};

H2CASE(file_writer, "append buffer")
{
    H2STUB(log_flusher_signal, fake_flusher_signal);

    mys_filewriter_t file_writer;
    log_buffer_t buffer1, buffer2;
    char buf1[20] = {0}, buf2[20] = {0};
    char *log = "this is a log test\n";

    memset(&file_writer, 0x0, sizeof(mys_filewriter_t));
    memset(&buffer1, 0x0, sizeof(log_buffer_t));
    memset(&buffer2, 0x0, sizeof(log_buffer_t));

    buffer1.buffer = buf1;
    buffer1.need_flush = 0;
    buffer1.len = 0;
    buffer1.size = 20;

    buffer2.buffer = buf2;
    buffer2.need_flush = 0;
    buffer2.len = 0;
    buffer2.size = 20;

    file_writer.buffers[0] = &buffer1;
    file_writer.buffers[1] = &buffer2;
    file_writer.using_bufidx = 0;
    file_writer.mutex = pltf_mutex_init();

    __append_buffer(&file_writer, log, strlen(log) + 1);

    H2EQ_MATH(strlen(log) + 1, buffer1.len);
    
    __append_buffer(&file_writer, log, strlen(log) + 1);

    H2EQ_MATH(strlen(log) + 1, buffer2.len);
    H2EQ_MATH(1, buffer1.need_flush);
    H2EQ_MATH(1, file_writer.using_bufidx);

    __append_buffer(&file_writer, log, strlen(log) + 1);

    H2EQ_MATH(1, buffer2.need_flush);
    H2EQ_MATH(1, file_writer.using_bufidx);
};

H2CASE(file_writer, "write log file")
{
    mys_logwriter_t log_writer;
    mys_filewriter_t file_writer;
    char *buff = "This is a test log to buffer\n";
    int len = strlen(buff) + 1;
    log_writer.priv = &file_writer;
    H2STUB(__flush_buffer, fake_flush_buffer);
    H2STUB(__append_buffer, fake_append_buffer);

    H2EQ_MATH(0,__write_log_file(&log_writer, buff, len, C_FLUSH_FULL));
    H2EQ_MATH(0,__write_log_file(&log_writer, buff, len, C_FLUSH_TIMEOUT));
};

H2CASE(file_writer, "prepare file writer")
{
    mys_filewriter_t *file_writer = (mys_filewriter_t *)malloc(sizeof(mys_filewriter_t));
    mys_logconfig_t  config;

    memset(&config, 0x0, sizeof(mys_logconfig_t));

    strcpy(config.dir, "./");
    strcpy(config.filename, "yunshang.log");
    config.buffer_size = 100;
    H2EQ_MATH(0, __prepare_file_writer(file_writer, &config));
    free(file_writer);
    __final_file_writer(file_writer);
};

H2CASE(file_writer, "final file writer")
{
    H2STUB(__flush_buffer, fake_flush_buffer);
    H2STUB(log_buffer_del, fake_log_buffer_del);

    mys_filewriter_t file_writer;
    memset(&file_writer, 0x0, sizeof(mys_filewriter_t));
    
    __final_file_writer(&file_writer);
};

H2CASE(file_writer,"initialize and finalize")
{
    //H2STUB(__final_file_writer,fake_final_file_writer);
    mys_logconfig_t config;
    strcpy(config.dir, "./");
    strcpy(config.filename, "yunshang.log");
    config.buffer_size = 100;

    mys_logwriter_t *log_writer = NULL;
    log_writer = file_writer_initialize(&config);
    H2EQ_TRUE(NULL != log_writer->priv);
    H2EQ_TRUE(NULL != log_writer->write);
    H2EQ_TRUE(NULL != log_writer);
    //H2EQ_MATH(0, force);

    file_writer_finalize(log_writer);
};

