#include "h2unit.h"
extern "C" {
#include "../../../../libmyslog/src/config/config.c"
}
#include <stdio.h>
#include <unistd.h>

H2UNIT(log_config)
{
    void setup() {
    }
    void teardown() {
    }
};

static char config_json[] =
    "{"
        "\"level\": 3, "
        "\"filelog\": \"file\","
        "\"catlog\": \"cat\","
        "\"consolelog\": \"console,all\","
        "\"netlog\": \"net\","
        "\"filename\": \"filename\","
        "\"directory\": \"directory\","
        "\"format\": \"format\","
        "\"host\": \"127.0.0.1\","
        "\"port\": 8080,"
        "\"buffersize\": 1024,"
        "\"force_flush\": 1,"
        "\"force_flush_seconds\": 5,"
        "\"force_flush_targets\": \"filelog\""
     "}";

static void __check_config_result(mys_logconfig_t *config)
{
    mys_logpriv_t *priv = (mys_logpriv_t *)config->priv;
    H2EQ_MATH(3, config->level);
    H2EQ_STRCMP("file", priv->filelog_tag);
    H2EQ_STRCMP("cat", priv->catlog_tag);
    H2EQ_STRCMP("console,all", priv->consolelog_tag);
    H2EQ_STRCMP("net", priv->netlog_tag);
    H2EQ_STRCMP("directory", config->dir);
    H2EQ_STRCMP("127.0.0.1", config->host);
    H2EQ_STRCMP("filename", config->filename);
    H2EQ_MATH(8080, config->port);
    H2EQ_MATH(1, config->force_flush_targets);
    H2EQ_MATH(5, config->force_flush_seconds);
    H2EQ_MATH(1, config->force_flush);
    H2EQ_MATH(1024, config->buffersize);
}

H2CASE(log_config, "parse config_targets")
{
    H2EQ_MATH(1, __force_outputs_mask("filelog"));
    H2EQ_MATH(4, __force_outputs_mask("netlog"));
    H2EQ_MATH(5, __force_outputs_mask("filelog,netlog"));
};

H2CASE(log_config, "parse config json")
{
    mys_logpriv_t priv;
    mys_logconfig_t config;

    memset(&priv, 0x0, sizeof(mys_logpriv_t));
    memset(&config, 0x0, sizeof(mys_logconfig_t));
    config.priv = &priv;

    __parse_config_json(config_json, &config);
    __check_config_result(&config);
    H2EQ_MATH(0, __parse_config_json(config_json, &config));
};

static void __prepare_config_file(char *filename, char *content)
{
    FILE *fp = fopen(filename, "w+");

    if (NULL == fp) {
        return;
    }
    fwrite(content, strlen(content), 1, fp);
    fclose(fp);
}

H2CASE(log_config, "read config file")
{
    char filename[] = "./myslog.conf";
    char config_read[512] = {0};
    int len = strlen(config_json);

    __prepare_config_file(filename, config_json);
    __read_config_file(filename, config_read, &len);
    H2EQ_STRCMP(config_json, config_read);
    H2EQ_MATH(len, strlen(config_json));
    unlink(filename);
};
H2CASE(log_config, "parse config")
{
    mys_logconfig_t config;
    mys_logpriv_t priv;
    memset(&config, 0x0, sizeof(mys_logconfig_t));
    memset(&priv, 0x0, sizeof(mys_logpriv_t));
    config.priv = &priv;
    char filename[] = "./myslog.conf";
    __prepare_config_file(filename, config_json);
    H2EQ_MATH(0, __parse_config(filename, &config));
    __check_config_result(&config);
};

H2CASE(log_config, "log config initialize")
{
    mys_logconfig_t *config = NULL;
    char filename[] = "./myslog.conf";

    __prepare_config_file(filename, config_json);

    config = log_config_initialize(filename);
    H2EQ_TRUE(NULL != config);

    if (NULL != config) {
        H2EQ_TRUE(NULL != config->priv);
    } else {
        return;
    }

    __check_config_result(config);
    log_config_finalize(config);
};

H2CASE(log_config, "output mask")
{
    mys_logpriv_t priv;
    memset(&priv, 0x0, sizeof(mys_logpriv_t));
    strcpy(priv.filelog_tag , "filelog");
    strcpy(priv.consolelog_tag , "all");
    H2EQ_MATH(3,  __outputs_mask(&priv) );
};
H2CASE(log_config, "output mask")
{
    char tag[10] = "filelog";
    mys_logpriv_t priv;
    memset(&priv, 0x0, sizeof(mys_logpriv_t));
    strcpy(priv.filelog_tag , "all");
    strcpy(priv.consolelog_tag , "all");
    H2EQ_MATH(3, __tag_outputs_mask(&priv, tag));
};
