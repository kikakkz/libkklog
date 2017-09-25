rootdir := $(shell pwd)/../../..
project := ubuntu-base

include $(rootdir)/build/common/inc.mk
ifeq ($(hostmaketype), windows)
rootdir := ../../..
endif

TARGET_LIB = libmyslog.$(staticlibsuffix)
MODULE = libmyslog
MODULEVERSION = 1.3.0

include ../source.mk

include $(rootdir)/build/utils/archive.$(hostmaketype).mk
