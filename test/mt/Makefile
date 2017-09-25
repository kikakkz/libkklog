rootdir := $(shell pwd)/../../..
project := ubuntu-base

include $(rootdir)/build/common/inc.mk
ifeq ($(hostmaketype), windows)
rootdir := ../../..
endif

####
##  Application targets
####
DEP_HEADERS += $(incdir)/libmyslog                                      \
               $(incdir)/cJSON                                          \
               $(incdir)/platform

apps = log_test
log_test_SRCS = $(rootdir)/libmyslog/test/mt/log_test.c


log_test_ARCHIVES = myslog platform cJSON
log_test_LDLIBS = $(PTHREAD_LDFLAGS) $(LDFLAGS_EXTRA) $(SYSTEM_LDLIBS) $(LIBRT_LDFLAGS)
log_test_DISTSOLIBS =


include $(rootdir)/build/utils/app.mk
