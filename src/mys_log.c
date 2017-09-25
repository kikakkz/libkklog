#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#include "pltf_thread.h"

#include "mys_log.h"

#include "config.h"
#include "log_tag.h"
#include "log_assemble.h"
#include "log_observer.h"
#include "log_flusher.h"

#define C_MAX_CONCURRENT_COUNT          32
#define C_MAX_THREADNAME_LEN            32
#define C_INVALID_THREAD_KEY            (void *)0xFFFFFFFF
#define C_THREAD_NAME_NA                "N/A"


typedef struct {
     void          *thread_key;
     char           thread_name[C_MAX_THREADNAME_LEN];
} mys_logthread_t;

typedef struct {
    mys_logthread_t    threads[C_MAX_CONCURRENT_COUNT];
    void               *mutex;
    int                initialized;
    mys_logconfig_t    *config;
    void               *observer;
} mys_logdesc_t;

static mys_logdesc_t   g_log_desc;

static char *__level_tostring(mys_loglevel_t level)
{
    switch (level) {
    case N_LOG_NONE:        return "N";
    case N_LOG_ERROR:       return "E";
    case N_LOG_WARNING:     return "W";
    case N_LOG_ACCESS:      return "A";
    case N_LOG_DEBUG:       return "D";
    case N_LOG_TRACE:       return "T";
    case N_LOG_INFO:        return "I";
    default:                return "N/A";
    }
}

void mys_log_set_thread(char *name)
{
    int i;
    void *thread_key = C_INVALID_THREAD_KEY;

    if (0 == g_log_desc.initialized) {
        return;
    }

    thread_key = pltf_thread_self();

    pltf_mutex_lock(g_log_desc.mutex);
    for (i = 0; i < C_MAX_CONCURRENT_COUNT; i++) {
        if (thread_key == g_log_desc.threads[i].thread_key) {
            snprintf(g_log_desc.threads[i].thread_name, C_MAX_THREADNAME_LEN, "%lx-%s", 0xffffff&((unsigned long)thread_key), name);
            break;
        }
        if (C_INVALID_THREAD_KEY == g_log_desc.threads[i].thread_key) {
            g_log_desc.threads[i].thread_key = thread_key;
            snprintf(g_log_desc.threads[i].thread_name, C_MAX_THREADNAME_LEN, "%lx-%s", 0xffffff&((unsigned long)thread_key), name);
            break;
        }
    }
    pltf_mutex_unlock(g_log_desc.mutex);
}

static char *__get_thread_name(void)
{
    int i;
    void *thread_key = C_INVALID_THREAD_KEY;

    if (0 == g_log_desc.initialized) {
        return (char *)C_THREAD_NAME_NA;
    }

    thread_key = pltf_thread_self();

    pltf_mutex_lock(g_log_desc.mutex);
    for (i = 0; i < C_MAX_CONCURRENT_COUNT; i++) {
        if (C_INVALID_THREAD_KEY == g_log_desc.threads[i].thread_key) {
            g_log_desc.threads[i].thread_key = thread_key;
            snprintf(g_log_desc.threads[i].thread_name, C_MAX_THREADNAME_LEN, "%lx", 0xffffff&((unsigned long)thread_key));
            break;
        }
        if (thread_key == g_log_desc.threads[i].thread_key) {
            break;
        }
    }
    pltf_mutex_unlock(g_log_desc.mutex);

    return (C_MAX_CONCURRENT_COUNT < i) ? (char *)C_THREAD_NAME_NA : 
                g_log_desc.threads[i].thread_name;
}

int mys_log_initialize(char *config_filename)
{
    int outputs_mask = 0;
    int i = 0;

    memset(&g_log_desc, 0x0, sizeof(mys_logdesc_t));
    if (NULL == (g_log_desc.config =
                log_config_initialize(config_filename))) {
        return -1;
    }

    outputs_mask = log_config_get_outputs(g_log_desc.config);

    if (NULL == (g_log_desc.observer =
                log_observer_initialize(outputs_mask, g_log_desc.config))) {
        goto L_OBSERVER_INIT_ERR;
    }


    g_log_desc.mutex = pltf_mutex_init();

    for (i = 0; i < C_MAX_CONCURRENT_COUNT; i++) {
        g_log_desc.threads[i].thread_key = C_INVALID_THREAD_KEY;
    }


    g_log_desc.initialized = 1;
    log_flusher_initialize(g_log_desc.config, g_log_desc.observer);

    return 0;

L_OBSERVER_INIT_ERR:
    log_config_finalize(g_log_desc.config);
    return -1;
}

void mys_log_finalize(void)
{
    if (0 == g_log_desc.initialized) {
        return;
    }

    g_log_desc.initialized = 0;
    pltf_mutex_destroy(g_log_desc.mutex);
    log_flusher_finalize();
    log_observer_finalize(g_log_desc.observer);
    log_config_finalize(g_log_desc.config);
    g_log_desc.config = NULL;
}

void mys_log_write(mys_loglevel_t level, const char *tags,
        char *function, int line, char *fmt, ...)
{
#define C_MAX_LOGASS_LEN    2048
#define C_MAX_LOG_LEN      1024
    mys_logmask_t output_mask = {0};
    mys_logtag_t tag;
    int i = 0;
    va_list args;
    char info[C_MAX_LOG_LEN];
    char log_str[C_MAX_LOGASS_LEN];
    mys_logcontent_t logs[C_MAX_ASSEMBLE_CNT];

    if (0 == g_log_desc.initialized ||
            g_log_desc.config->level < level) {
        return;
    }

    memset(logs, 0, sizeof(mys_logcontent_t) * C_MAX_ASSEMBLE_CNT);
    va_start(args, fmt);
    vsnprintf(info, C_MAX_LOG_LEN, fmt, args);
    va_end(args);
    logs[N_ASS_WOPREFIX].content = info;
    logs[N_ASS_WOPREFIX].length = strlen(info);

    log_assemble(log_str, &tag, __level_tostring(level), tags,
            function, line, __get_thread_name(), info);
    logs[N_ASS_WPREFIX].content = log_str;
    logs[N_ASS_WPREFIX].length = strlen(log_str);

    for (i = 0; i < tag.count; i++) {
        log_config_get_tag_outputs(g_log_desc.config, &output_mask, tag.tags[i]);
    }

    log_observer_output(g_log_desc.observer, &output_mask, logs, 0);
}
