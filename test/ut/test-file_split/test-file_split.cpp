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

H2CASE(file_writer, "assemble new filename")
{
    mys_filewriter_t filewriter;
    strcpy(filewriter.dir, ".");
    strcpy(filewriter.filename, "yunshang");
    H2EQ_TRUE(0 == __assemble_new_filename(&filewriter));
    printf("currfile file is %s\n", filewriter.currfile);
};

H2CASE(file_writer, "recreate log file")
{
    mys_filewriter_t filewriter;
    memset(&filewriter, 0, sizeof(filewriter));
    strcpy(filewriter.dir, ".");
    strcpy(filewriter.filename, "yunshang");
    filewriter.fd = -1;
    filewriter.reserve_cnt = 2;

    __malloc_reserve_filename(&filewriter);

    filewriter.using_fidx = 0;


    H2EQ_TRUE(filewriter.fd == -1);
    H2EQ_MATH(0, __recreate_log_file(&filewriter));
    H2EQ_TRUE(filewriter.fd >= 0);
    H2EQ_TRUE(1 == filewriter.using_fidx);
    H2EQ_STRCMP(filewriter.currfile, filewriter.reserve_filename[filewriter.using_fidx]);

    pltf_file_close(filewriter.fd);

    __free_reserve_filename(&filewriter);
}
