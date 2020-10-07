# Cel Demo Makefile
# Copyright (c) 2008 - 2018 Hu Jinya <hu_jinya@163.com>

CROSS =
CXX = $(CROSS)gcc
STRIP = $(CROSS)strip

CXXFLAGS = -Wall -Wno-deprecated -fPIC  -g  -I ../../cel/include -I ../../rdp/include
LDFLAGS = 
LIBS = ../../cel/lib/libcel.a -lpthread -lcrypt -lssl -lcrypto

A_TARGET = 
SO_TARGET = 
BIN_TARGET = ../bin/cel-demo

MAJOR = 1
MINOR = 0
REVISION = 0
BUILD = 0
EXTRA = ""
VERSION_FILE="version.h"
.PHONY:$(VERSION_FILE)

SUB_DIRS =
SRC_DIRS = 
SRC_EXTS = .c 
OBJ_DIR = ../obj/src

include ../tmpl.mk