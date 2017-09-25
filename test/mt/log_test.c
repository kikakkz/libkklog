#include <pthread.h>
#include <stdio.h>

#include "mys_log.h"
#include "pltf_thread.h"
#include "pltf_time.h"

int running = 0;

void _logout1();
void _logout2();
void _logout3();

int main(int argc, char *argv[])
{
    char config_filename[] = "./conf/myslog.conf";
    void *id2, *id3, *id1;
    mys_log_initialize(config_filename);
    printf("initialize\n");
    running = 1;
    id1 = pltf_thread_create((void *)_logout1,NULL);
    id2 = pltf_thread_create((void *)_logout2,NULL);
    id3 = pltf_thread_create((void *)_logout3,NULL);
    //pltf_msleep(3 * 1000);
    while(1);
    running = 0;
    pltf_thread_join(id1);
    pltf_thread_join(id2);
    pltf_thread_join(id3);
    mys_log_finalize();
    return 0;
}

void _logout1()
{
    int i = 0;
    char *tag1 = "business,tag2,tag3";
    while (running){
        LOGE(tag1, (char *)"this is thread 1 log ,number = %d ",i);
    }
}

void _logout2()
{
    int i = 1;

    mys_log_set_thread("thread 2");
    while (running) {
        LOGW("monitor, test", "this is thread 2 log ,number = %d,%d", i, i);
    }
}

void _logout3()
{
    while (running) {
        BLOG("[p2pcloud] p2p_core_exit successfully!");
    }
}
