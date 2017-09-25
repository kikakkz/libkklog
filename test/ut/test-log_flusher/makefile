rootdir := $(shell pwd)/../../../..
project := ubuntu-base
MOD = libmyslog
H2OPTIONS = -v

include $(rootdir)/build/common/inc.mk

DEP_HEADERS = $(rootdir)/libmyslog/inc              \
              $(rootdir)/libmyslog/src/assemble     \
              $(rootdir)/libmyslog/src/utils        \
              $(rootdir)/libmyslog/src/config       \
              $(rootdir)/libmyslog/src/observer     \
              $(rootdir)/libmyslog/src/flusher      \
              $(rootdir)/libmyslog/src/buffer       \
              $(incdir)/cJSON                       \
              $(incdir)/platform

DEP_ARCHIVES = platform myslog
STD_LDFLAGS =-pthread

SRC_DIS =

SRC_FILES =

TEST_FILE =
TEST_SRC_DIRS =
TEST_SRC_FILES = test-log_flusher.cpp

include $(rootdir)/build/utils/unitest.mk
