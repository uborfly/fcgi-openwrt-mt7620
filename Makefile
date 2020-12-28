CC = mipsel-openwrt-linux-gcc

SUBDIRS=$(shell ls -l | grep ^d | awk '{if($$9 != "build" && $$9 != "inc") print $$9}')
BUILD=$(shell ls -l | grep ^d | awk '{if($$9 == "build") print $$9}')

ROOT_DIR=$(shell pwd)

BIN = upload

OBJS_DIR = build/objs
BIN_DIR = build/bin

CUR_SOURCE=${wildcard *.c}

CUR_OBJS=${patsubst %.c, %.o, $(CUR_SOURCE)}

export CC BIN OBJS_DIR BIN_DIR ROOT_DIR

all:$(SUBDIRS) $(CUR_OBJS) BUILD

$(SUBDIRS):ECHO
	make -C $@
BUILD:ECHO
#直接去build目录下执行makefile文件
	make -C build
ECHO:
	@echo $(SUBDIRS)
#将c文件编译为o文件，并放在指定放置目标文件的目录中即OBJS_DIR
$(CUR_OBJS):%.o:%.c
	$(CC) -c $^ -o $(ROOT_DIR)/$(OBJS_DIR)/$@


$(info START BUILD)

.PHONY:clean
clean:
	@rm $(OBJS_DIR)/*.o
	@rm -rf $(BIN_DIR)/*
