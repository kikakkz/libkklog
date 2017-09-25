#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/prctl.h>
#include "pltf_time.h"
#include "pltf_thread.h"

#include "log_flusher.h"
#include "config.h"
#include "log_observer.h"

#define SET_THREAD_NAME(name)           \
        do{                             \
            char buf[128];              \
            sprintf(buf, "%s", name);   \
            prctl(PR_SET_NAME, buf);    \
        } while(0);

static int        g_flusher_running = 0;
static void      *g_flusher_thread = NULL;
static void      *g_flusher_cond = NULL;
static void      *g_flusher_mutex = NULL;

typedef struct {
    void            *observer;
    mys_logconfig_t *config;
} mys_flusherarg_t;

static void *__flusher(void *arg)
{
    mys_flusherarg_t *flusher_arg = (mys_flusherarg_t *)arg;
    mys_logconfig_t *config = flusher_arg->config;
    void *observer = flusher_arg->observer;
    int ret = 0;
    int flush_type = C_FLUSH_FULL;
    SET_THREAD_NAME("log_flusher");

    while (g_flusher_running) {

        if (ETIMEDOUT == ret) {
            flush_type = C_FLUSH_TIMEOUT;
        } else {
            flush_type = C_FLUSH_FULL;
        }

        log_observer_output(observer,
                &config->force_flush_targets, NULL, flush_type);

        pltf_mutex_lock(g_flusher_mutex);
        ret = pltf_cond_timedwait(g_flusher_cond, g_flusher_mutex,
                config->force_flush_seconds * 1000);
        pltf_mutex_unlock(g_flusher_mutex);
    }

    free(arg);
    return NULL;
}

void log_flusher_initialize(mys_logconfig_t *config, void *observer)
{
    mys_flusherarg_t *arg =
        (mys_flusherarg_t *)malloc(sizeof(mys_flusherarg_t));

    if (NULL == arg) {
        goto L_ERROR;
    }

    if (NULL == (g_flusher_mutex = pltf_mutex_init())) {
        goto L_ERROR_MUTEX;
    }

    if (NULL == (g_flusher_cond = pltf_cond_init())) {
        goto L_ERROR_COND;
    }

    arg->config = config;
    arg->observer = observer;

    g_flusher_running = 1;
    if (NULL == (g_flusher_thread =
                pltf_thread_create(__flusher, arg))) {
        goto L_ERROR_CREATE;
    }

    return;

L_ERROR_CREATE:
    pltf_cond_destroy(g_flusher_cond);
L_ERROR_COND:
    pltf_mutex_destroy(g_flusher_mutex);
L_ERROR_MUTEX:
    free(arg);
L_ERROR:
    return;
}

void log_flusher_finalize(void)
{
    if (g_flusher_running) {
        g_flusher_running = 0;
        log_flusher_signal();
        pltf_thread_join(g_flusher_thread);
    }

    pltf_mutex_destroy(g_flusher_mutex);
    pltf_cond_destroy(g_flusher_cond);
}

int log_flusher_signal(void)
{
    int ret = 0;
    pltf_mutex_lock(g_flusher_mutex);
    ret = pltf_cond_signal(g_flusher_cond);
    pltf_mutex_unlock(g_flusher_mutex);
    return ret;
}
