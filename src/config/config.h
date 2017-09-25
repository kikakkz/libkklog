#ifndef __MYS_LOG_CONFIG_H__
#define __MYS_LOG_CONFIG_H__

#include "mys_log.h"

#define C_MAX_LOG_FILENAME_LEN      32
#define C_MAX_LOG_DIRNAME_LEN       256
#define C_MAX_LOG_HOST_LEN          64
#define C_MAX_FILELOG_CNT           5
#define C_MAX_LOGWRITER_COUNT       4

#define C_FULL_MASK                 0xFFFFFFFF
#define C_EMPTY_MASK                0
#define C_MAX_ASSEMBLE_CNT          2

typedef enum {
    N_ASS_WOPREFIX,
    N_ASS_WPREFIX
} mys_assemble_t;

typedef enum {
    N_LOGOUTPUT_FILE,
    N_LOGOUTPUT_CONSOLE,
    N_LOGOUTPUT_NET,
    N_LOGOUTPUT_CATLOG
} mys_logoutput_t;

#define C_LOG_FILE_OUTPUT           (1 << N_LOGOUTPUT_FILE)
#define C_LOG_CONSOLE_OUTPUT        (1 << N_LOGOUTPUT_CONSOLE)
#define C_LOG_NET_OUTPUT            (1 << N_LOGOUTPUT_NET)
#define C_LOG_CATLOG_OUTPUT         (1 << N_LOGOUTPUT_CATLOG)

typedef struct {
    char  *content;
    int    length;
} mys_logcontent_t;

typedef struct {
    int output_mask;
    int fd_mask[C_MAX_LOGWRITER_COUNT];
} mys_logmask_t;

typedef struct {
    char            filename[C_MAX_LOG_FILENAME_LEN];
    char            dir[C_MAX_LOG_DIRNAME_LEN];
    unsigned long   split_size;
    int             reserve_cnt;
    void           *priv;
    mys_assemble_t  assemble;
} file_config_t;

typedef struct {
    file_config_t   files[C_MAX_FILELOG_CNT];
    int             file_cnt;
    int             buffer_size;
} filelog_config_t;

typedef struct {
    void            *priv;
    mys_assemble_t   assemble;
} console_config_t;

typedef struct {
    char            host[C_MAX_LOG_HOST_LEN];
    int             port;
    int             buffer_size;
    mys_assemble_t  assemble;
    void           *priv;
} netlog_config_t;

typedef struct {
    void           *priv;
} cat_config_t;

typedef struct {
    filelog_config_t    fileconfig;
    netlog_config_t     netconfig;
    console_config_t    consoleconfig;
    cat_config_t        catconfig;
    mys_loglevel_t      level;
    int                 force_flush_seconds;
    mys_logmask_t       force_flush_targets;
} mys_logconfig_t;

#ifdef __cplusplus
extern "C" {
#endif

mys_logconfig_t *log_config_initialize(char *config_filename);
void             log_config_finalize(mys_logconfig_t *config);

int              log_config_get_outputs(mys_logconfig_t *config);
int              log_config_get_tag_outputs(mys_logconfig_t *config, mys_logmask_t *tag_mask, char *tag);

#ifdef __cplusplus
}
#endif

#endif
