#ifndef __MYS_LOG_H__
#define __MYS_LOG_H__

typedef enum {
    N_LOG_NONE,
    N_LOG_ERROR,
    N_LOG_WARNING,
    N_LOG_ACCESS,
    N_LOG_DEBUG,
    N_LOG_TRACE,
    N_LOG_INFO,
} mys_loglevel_t;

#ifdef __cplusplus
extern "C" {
#endif


int  mys_log_initialize(char *config_filename);

void mys_log_set_thread(char *name);
void mys_log_write(mys_loglevel_t level, const char *tags,
        char *function, int line, char *fmt, ...);

void mys_log_finalize(void);

#ifdef __cplusplus
}
#endif

#define LOG(level, tag, fmt, ...) \
    do { \
        mys_log_write(level, tag, (char *)__FUNCTION__, __LINE__, fmt"\n", ##__VA_ARGS__); \
    } while (0)

#define LOGE(tag, fmt, ...) LOG(N_LOG_ERROR, tag, fmt, ##__VA_ARGS__)
#define LOGW(tag, fmt, ...) LOG(N_LOG_WARNING, tag, fmt, ##__VA_ARGS__)
#define LOGA(tag, fmt, ...) LOG(N_LOG_ACCESS, tag, fmt, ##__VA_ARGS__)
#define LOGD(tag, fmt, ...) LOG(N_LOG_DEBUG, tag, fmt, ##__VA_ARGS__)
#define LOGT(tag, fmt, ...) LOG(N_LOG_TRACE, tag, fmt, ##__VA_ARGS__)
#define DBG(fmt, ...)       LOG(N_LOG_DEBUG, "debug", fmt, ##__VA_ARGS__)
#define ERR(fmt, ...)       LOG(N_LOG_ERROR, "error", fmt, ##__VA_ARGS__)

#define IGN(...)            do { } while (0)
#define BLOG(fmt, ...)      LOG(N_LOG_INFO, "boot", fmt, ##__VA_ARGS__)
#endif

