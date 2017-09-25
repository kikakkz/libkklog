COMMON_INCLUDE_DIRS += $(rootdir)/$(MODULE)/inc                         \
                       $(rootdir)/$(MODULE)/src/observer                \
                       $(rootdir)/$(MODULE)/src/assemble                \
                       $(rootdir)/$(MODULE)/src/config                  \
                       $(rootdir)/$(MODULE)/src/utils                   \
                       $(rootdir)/$(MODULE)/src/flusher                 \
                       $(rootdir)/$(MODULE)/src/buffer                  \
                       $(rootdir)/$(MODULE)/src                         \
                       $(incdir)/platform                               \
                       $(incdir)/cJSON

COMMON_SRC_FILES := $(wildcard $(rootdir)/$(MODULE)/src/*.c)            \
                    $(wildcard $(rootdir)/$(MODULE)/src/assemble/*.c)   \
                    $(wildcard $(rootdir)/$(MODULE)/src/config/*.c)     \
                    $(wildcard $(rootdir)/$(MODULE)/src/utils/*.c)      \
                    $(wildcard $(rootdir)/$(MODULE)/src/observer/*.c)   \
                    $(wildcard $(rootdir)/$(MODULE)/src/buffer/*.c)   \
                    $(wildcard $(rootdir)/$(MODULE)/src/flusher/*.c)

COMMON_INST_HEADER_DIRS += $(rootdir)/$(MODULE)/inc

COMMON_INST_EXTRA_DIRS += $(rootdir)/$(MODULE)/extra
