# Makefile templete
# Copyright (c) 2008 - 2017 Hu Jinya <hu_jinya@163.com>

#######################################################################################
# �Զ��岿�֣�������Ŀʵ�������޸�
#######################################################################################
# ��������Ϣ
# CROSS =
# CXX = $(CROSS)g++
# STRIP = $(CROSS)strip

# ����ѡ��,��-I ../../cel/include
# CXXFLAGS = -Wall -Wno-deprecated -fPIC  -g  -I ../../cel/include
# ����ѡ���-L/usr/lib64/mysql
# LDFLAGS = -L/usr/lib64/mysql
# ������,�� -lcrypt -lssl -lcrypto ../../cel/lib/libcel.a
# LIBS = -lpthread -lmysqlclient -lcrypt -lssl -lcrypto ../../cel/lib/libcel.a

# ��̬���ļ�����ָ��Ŀ¼����../lib/libcel.a
# A_TARGET = 
# ��̬���ļ�����ָ��Ŀ¼����../lib/libcel.so
# SO_TARGET =
# ��ִ���ļ�����ָ��Ŀ¼����../bin/cel-example
# BIN_TARGET = 

# ����汾��Ϣ�������Զ�����"version.h"ͷ�ļ���������Ҫ��ȫ����ֵΪ�ռ���
# MAJOR = 0
# MINOR = 0
# REVISION = 0
# BUILD = 0
# EXTRA = ""
# VERSION_FILE= "version.h"

# ģ����Ŀ¼
# SUB_DIRS = 
# ��ǰģ��Դ����Ŀ¼
# SRC_DIRS = 
# Դ���׺
# SRC_EXTS = .c .cc .cpp .cxx 
# ����������м��ļ�����Ŀ¼
# OBJ_DIR = ../obj/example

#######################################################################################
# �������֣�����Ҫ�޸�
#######################################################################################
SRCS = $(wildcard $(addprefix *,$(SRC_EXTS)))
SRCS += $(foreach dir,$(SRC_DIRS),$(wildcard $(addprefix $(dir)/*,$(SRC_EXTS))))
DEPS = $(addprefix $(OBJ_DIR)/,$(addsuffix .d, $(basename $(SRCS))))
OBJS = $(addprefix $(OBJ_DIR)/,$(addsuffix .o, $(basename $(SRCS))))

MAKE_OBJ_DIR := $(shell mkdir -p $(OBJ_DIR) \
$(addprefix $(OBJ_DIR)/,$(SUB_DIRS)) $(addprefix $(OBJ_DIR)/,$(SRC_DIRS)))

.PHONY : strip install clean show

all:SUBDIRS $(VERSION_FILE) $(A_TARGET) $(SO_TARGET) $(BIN_TARGET)

SUBDIRS:$(SUB_DIRS)
	@for dir in $(SUB_DIRS);\
	do $(MAKE) -C $$dir all||exit 1;\
	done
	
$(VERSION_FILE):
	@echo \#ifndef __VERSION_H__ >.ver; \
	echo \#define __VERSION_H__ >>.ver; \
	echo \#define MAJOR $(MAJOR) >> .ver; \
	echo \#define MINOR $(MINOR) >> .ver; \
	echo \#define REVISION $(REVISION) >> .ver; \
	echo \#define BUILD $(BUILD) >> .ver; \
	echo \#define EXTRA \"$(EXTRA)\" >> .ver; \
	echo \#define OS \"`head -n 1 /etc/issue`\">> .ver; \
	echo \#define PLATFORM \"`uname -sr`\" >> .ver; \
	echo \#define COMPILE_BY \"`whoami`\" >> .ver; \
	echo \#define COMPILE_HOST \"`hostname`\">> .ver; \
	echo \#define COMPILER \"`$(CXX) -v 2>&1 | tail -1`\" >> .ver; \
	echo \#endif >>.ver; \
	mv -f .ver $@

$(A_TARGET): $(OBJS)
	$(AR) cr $@ $^

$(SO_TARGET): $(OBJS)
	$(CXX) -shared -fPIC -o $@ $^ $(LDFLAGS) $(LIBS)

$(BIN_TARGET): $(OBJS) $(VERSION_FILE)
	$(CXX) -o $@ $^ $(LDFLAGS) $(LIBS)

$(OBJ_DIR)/%.o: %.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<
$(OBJ_DIR)/%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<
$(OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
$(OBJ_DIR)/%.o: %.cxx
	$(CXX) $(CXXFLAGS) -c -o $@ $<
	
$(OBJ_DIR)/%.d: %.c
	@echo "Creating dependency file \"$@ <- $^\"."
	@$(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\(.*\)\.o[ :]*,$(OBJ_DIR)/$*.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.*
$(OBJ_DIR)/%.d: %.cc
	@echo "Creating dependency file \"$@ <- $^\"."
	@$(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\(.*\)\.o[ :]*,$(OBJ_DIR)/$*.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.*
$(OBJ_DIR)/%.d: %.cpp
	@echo "Creating dependency file \"$@ <- $^\"."
	@$(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\(.*\)\.o[ :]*,$(OBJ_DIR)/$*.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.*
$(OBJ_DIR)/%.d: %.cxx
	@echo "Creating dependency file \"$@ <- $^\"."
	@$(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\(.*\)\.o[ :]*,$(OBJ_DIR)/$*.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.*

-include $(DEPS)

# strip
strip:
	@for dir in $(SUB_DIRS);\
	do $(MAKE) -C $$dir strip||exit 1;\
	done
ifneq ($(SO_TARGET)_$(BIN_TARGET), ) 
	$(STRIP) $(SO_TARGET) $(BIN_TARGET)
endif

# install
install:
	@for dir in $(SUB_DIRS);\
	do $(MAKE) -C $$dir install||exit 1;\
	done
ifneq ($(SO_TARGET)_$(BIN_TARGET), ) 
	$(STRIP) $(SO_TARGET) $(BIN_TARGET)
	cp -f $(SO_TARGET) $(LIB_INSTALL_PATH)
endif

# clean
clean:
	@for dir in $(SUB_DIRS);\
	do $(MAKE) -C $$dir clean||exit 1;\
	done
ifneq ($(A_TARGET)$(SO_TARGET)$(BIN_TARGET), ) 
	rm -f $(DEPS) $(OBJS) $(A_TARGET) $(SO_TARGET) $(BIN_TARGET)
	rm -rf $(OBJ_DIR)
endif

# Show variables (for debug use only.)
show:
	@echo 'SUB_DIRS   :' $(SUB_DIRS)
	@echo 'SRC_DIRS   :' $(SRC_DIRS)
	@echo 'SRCS       :' $(SRCS)
	@echo 'DEPS       :' $(DEPS)
	@echo 'OBJS       :' $(OBJS)
	@echo 'A_TARGET   :' $(A_TARGET)
	@echo 'SO_TARGET  :' $(SO_TARGET)
	@echo 'BIN_TARGET :' $(BIN_TARGET)
