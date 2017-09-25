#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "cJSON.h"

#define C_DEFAULT_LOGCONF_FILENAME  "myslog.conf"
#define C_DEFAULT_LOGCONF_DIRNAME   "./conf"
#define C_DEFAULT_FLUSH_SECONDS      10

#define C_MAX_LOGTAG_LEN            4096
#define C_MAX_LOGCONFIG_LEN         4096

#define C_DEFAULT_APPENDER          MYS_LOG_FILE_APPENDER

typedef struct {
    char    tag[C_MAX_LOGTAG_LEN];
    int     output_all;
} log_configpriv_t;

static mys_logconfig_t   g_log_config;

static int __load_default_config(mys_logconfig_t *config)
{
    config->force_flush_seconds = C_DEFAULT_FLUSH_SECONDS;
    memset(&config->force_flush_targets, 0, sizeof(mys_logmask_t));
    return 0;
}

static int __force_flush_mask(mys_logconfig_t *config)
{
    mys_logmask_t *mask = &(config->force_flush_targets);
    /* file force flush */
    mask->output_mask |= C_LOG_FILE_OUTPUT;
    mask->fd_mask[N_LOGOUTPUT_FILE] = C_FULL_MASK;
    /* net force flush */
    mask->output_mask |= C_LOG_NET_OUTPUT;

    return 0;
}

static int __outputs_mask(mys_logconfig_t *config)
{
    int mask = 0, i = 0;
    log_configpriv_t *priv = NULL;

    for (i = 0; i < config->fileconfig.file_cnt; i++) {
        priv = config->fileconfig.files[i].priv;
        if (0 != strlen(priv->tag)) {
            mask |= C_LOG_FILE_OUTPUT;
        }
        if (NULL != strstr(priv->tag, "all")) {
            priv->output_all = 1;
        }
    }

    priv = config->consoleconfig.priv;
    if (0 != strlen(priv->tag)) {
        mask |= C_LOG_CONSOLE_OUTPUT;
    }
    if (NULL != strstr(priv->tag, "all")) {
        priv->output_all = 1;
    }

    priv = config->netconfig.priv;
    if (0 != strlen(priv->tag)) {
        mask |= C_LOG_NET_OUTPUT;
    }
    if (NULL != strstr(priv->tag, "all")) {
        priv->output_all = 1;
    }

    priv = config->catconfig.priv;
    if (0 != strlen(priv->tag)) {
        mask |= C_LOG_CATLOG_OUTPUT;
    }
    if (NULL != strstr(priv->tag, "all")) {
        priv->output_all = 1;
    }
    
    return mask;
}

int log_config_get_outputs(mys_logconfig_t *config)
{
    if (NULL == config) {
        return 0;
    }

    return __outputs_mask(config);
}

static int __tag_outputs_mask(mys_logconfig_t *config, mys_logmask_t *log_mask, char *tag)
{
    int i = 0;
    log_configpriv_t *priv = NULL;

    for (i = 0; i < config->fileconfig.file_cnt; i++) {
        priv = config->fileconfig.files[i].priv;
        if ((1 == priv->output_all) ||
                (NULL != strstr(priv->tag, tag))) {
            log_mask->output_mask |= C_LOG_FILE_OUTPUT;
            log_mask->fd_mask[N_LOGOUTPUT_FILE] |= (1 << i);
        }
    }

    priv = config->consoleconfig.priv;
    if (NULL != strstr(priv->tag, tag) ||
            1 == priv->output_all) {
        log_mask->output_mask |= C_LOG_CONSOLE_OUTPUT;
    }

    priv = config->netconfig.priv;
    if (NULL != strstr(priv->tag, tag) ||
            1 == priv->output_all) {
    }

    priv = config->catconfig.priv;
    if (NULL != strstr(priv->tag, tag) ||
            1 == priv->output_all) {
    }

    return 0;
}

int log_config_get_tag_outputs(mys_logconfig_t *config, mys_logmask_t *output_mask, char *tag)
{
    if (NULL == config || NULL == output_mask || NULL == tag) {
        return 0;
    }

    __tag_outputs_mask(config, output_mask, tag);

    return 0;
}

static int __read_config_file(char *config_filename, char *config_str, int *len)
{
    FILE *file = NULL;
    long filesize = 0;
    int ret = 0;

    if (NULL == (file = fopen(config_filename, "r"))) {
        return -1;
    }

    fseek(file, 0, SEEK_END);
    filesize = ftell(file);

    if (*len < filesize) {
        ret = -1;
        goto END;
    }

    fseek(file, 0, SEEK_SET);
    if ((filesize = fread(config_str, sizeof(char), filesize, file)) < 0) {
        ret = -1;
        goto END;
    }

    ret = *len = filesize;

END:
    fclose(file);
    return ret;
}

static void __parse_file_config_json(cJSON *file_json, file_config_t *file_config)
{
    cJSON *value = NULL;
    log_configpriv_t  *priv = NULL;
    char *v_str = NULL;

    if (NULL == file_json || NULL == file_config) {
        return;
    }

    value = cJSON_GetObjectItem(file_json, "filename");
    if (value) {
        if (NULL != (v_str = value->valuestring)) {
            strcpy(file_config->filename, v_str);
        }
    }

    value = cJSON_GetObjectItem(file_json, "directory");
    if (value) {
        if (NULL != (v_str = value->valuestring)) {
            strcpy(file_config->dir, v_str);
        }
    }

    value = cJSON_GetObjectItem(file_json, "tags");
    if (value) {
        priv = file_config->priv;
        if (NULL != (v_str = value->valuestring)) {
            strcpy(priv->tag, v_str);
        }
    }

    value = cJSON_GetObjectItem(file_json, "assemble");
    if (value) {
        file_config->assemble = (mys_assemble_t)value->valueint;
    }

    value = cJSON_GetObjectItem(file_json, "reserve_cnt");
    if (value) {
        file_config->reserve_cnt = value->valueint;
    }

    value = cJSON_GetObjectItem(file_json, "split_size");
    if (value) {
        file_config->split_size = value->valueint * 1024 * 1024;
    }

    return;
}
static int __parse_config_json(char *config_json, mys_logconfig_t *config)
{
    cJSON *json = NULL, *jarray = NULL, *item = NULL;
    cJSON *value = NULL;
    char  *v_str = NULL;
    int file_cnt = 0, i = 0;
    log_configpriv_t  *priv = NULL;

    if (NULL == (json = cJSON_Parse(config_json))) {
        return -1;
    }
    /* common config parameters */
    value = cJSON_GetObjectItem(json, "level");
    if (value) {
        config->level = (mys_loglevel_t )value->valueint;
    }

    value = cJSON_GetObjectItem(json, "force_flush_seconds");
    if (value) {
        config->force_flush_seconds = value->valueint;
        if (0 == config->force_flush_seconds) {
            config->force_flush_seconds = C_DEFAULT_FLUSH_SECONDS;
        }
    }

    value = cJSON_GetObjectItem(json, "buffer_size");
    if (value) {
        config->fileconfig.buffer_size = value->valueint;
        config->netconfig.buffer_size = value->valueint;
    }

    /* filelog config parameters */

    jarray = cJSON_GetObjectItem(json, "filelog");
    if (jarray) {
       file_cnt = cJSON_GetArraySize(jarray);
       if (file_cnt > C_MAX_FILELOG_CNT) {
            file_cnt = C_MAX_FILELOG_CNT;
       }
       config->fileconfig.file_cnt = file_cnt;
       for (i = 0; i < file_cnt; i++) {
            item = cJSON_GetArrayItem(jarray, i);
            if (item) {
                if (NULL == (config->fileconfig.files[i].priv =
                            (log_configpriv_t *)malloc(sizeof(log_configpriv_t)))) {
                    goto L_ERROR;
                }
                memset(config->fileconfig.files[i].priv, 0, sizeof(log_configpriv_t));
                __parse_file_config_json(item, &config->fileconfig.files[i]);
            }
       }
    }

    /* netlog config parameters */
    value = cJSON_GetObjectItem(json, "netlog");
    if (value) {
        priv = config->netconfig.priv;
        if (NULL != (v_str = value->valuestring)) {
            strcpy(priv->tag, v_str);
        }
    }
    value = cJSON_GetObjectItem(json, "host");
    if (value) {
        if (NULL != (v_str = value->valuestring)) {
            strcpy(config->netconfig.host, v_str);
        }
    }

    value = cJSON_GetObjectItem(json, "port");
    if (value) {
        config->netconfig.port = value->valueint;
    }

    value = cJSON_GetObjectItem(json, "consolelog");
    if (value) {
        priv = config->consoleconfig.priv;
        if (NULL != (v_str = value->valuestring)) {
            strcpy(priv->tag, v_str);
        }
    }

    value = cJSON_GetObjectItem(json, "catlog");
    if (value) {
        priv = config->catconfig.priv;
        if (NULL != (v_str = value->valuestring)) {
            strcpy(priv->tag, v_str);
        }
    }

    __force_flush_mask(config);

    cJSON_Delete(json);

    return 0;

L_ERROR:
    for (i = 0; i < config->fileconfig.file_cnt; i++) {
        if (config->fileconfig.files[i].priv) {
            free(config->fileconfig.files[i].priv);
        }
    }
    config->fileconfig.file_cnt = 0;

    return -1;
}

static int __parse_config(char *config_filename, mys_logconfig_t *config)
{
    char config_json[C_MAX_LOGCONFIG_LEN] = {0};
    int len = C_MAX_LOGCONFIG_LEN;

    if (__read_config_file(config_filename, config_json,
                &len) < 0) {
        return -1;
    }
    if (__parse_config_json(config_json, config) < 0) {
        return -1;
    }
    return 0;
}

static int __prepare_log_config(mys_logconfig_t *config)
{
    if (NULL == config) {
        goto L_ERROR_CONFIG;
    }
    memset(config, 0, sizeof(mys_logconfig_t));
    if (NULL == (config->consoleconfig.priv =
                (log_configpriv_t *)malloc(sizeof(log_configpriv_t)))) {
        goto L_ERROR_CONSOLE;
    }
    memset(config->consoleconfig.priv, 0, sizeof(log_configpriv_t));

    if (NULL ==(config->netconfig.priv =
                (log_configpriv_t *)malloc(sizeof(log_configpriv_t)))) {
        goto L_ERROR_NET;
    }
    memset(config->netconfig.priv, 0, sizeof(log_configpriv_t));

    if (NULL == (config->catconfig.priv =
                (log_configpriv_t *)malloc(sizeof(log_configpriv_t)))) {
        goto L_ERROR_CAT;
    }
    memset(config->catconfig.priv, 0, sizeof(log_configpriv_t));

    return 0;

L_ERROR_CAT:
    free(config->netconfig.priv);
L_ERROR_NET:
    free(config->consoleconfig.priv);
L_ERROR_CONSOLE:
L_ERROR_CONFIG:
    return -1;
}
mys_logconfig_t *log_config_initialize(char *config_filename)
{
    if (-1 == __prepare_log_config(&g_log_config)) {
        goto L_ERROR;
    }
    if (NULL == config_filename) {
        config_filename = C_DEFAULT_LOGCONF_DIRNAME"/"C_DEFAULT_LOGCONF_FILENAME;
    }
    if (__parse_config(config_filename, &g_log_config) < 0) {
        if (__load_default_config(&g_log_config) < 0) {
            goto L_ERROR;
        }
    }
    return &g_log_config;

L_ERROR:
    return NULL;
}

static void __final_log_config(mys_logconfig_t *config)
{
    int i = 0;
    if (NULL == config) {
        return;
    }
    /* filelog config free */
    for (i = 0; i < config->fileconfig.file_cnt; i++) {
        if (config->fileconfig.files[i].priv) {
            free(config->fileconfig.files[i].priv);
            config->fileconfig.files[i].priv = NULL;
        }
    }

    if (config->consoleconfig.priv) {
        free(config->consoleconfig.priv);
        config->consoleconfig.priv = NULL;
    }

    if (config->netconfig.priv) {
        free(config->netconfig.priv);
        config->netconfig.priv = NULL;
    }

    if (config->catconfig.priv) {
        free(config->catconfig.priv);
        config->catconfig.priv = NULL;
    }
}
void log_config_finalize(mys_logconfig_t *config)
{
    if (NULL != config) {
        __final_log_config(config);
    }
}
