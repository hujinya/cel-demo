# SLB Makefile
# Copyright (c) 2008 - 2017 Hu Jinya <hu_jinya@163.com>

CROSS =
CXX = $(CROSS)gcc
STRIP = $(CROSS)strip

CXXFLAGS =
LDFLAGS = 
LIBS = 

A_TARGET =
SO_TARGET =
BIN_TARGET = 

MAJOR = 
MINOR = 
REVISION = 
BUILD = 
EXTRA = 
VERSION_FILE=

SUB_DIRS = src
SRC_DIRS = 
SRC_EXTS = 
OBJ_DIR = ../obj

include ./tmpl.mk