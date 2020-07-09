DIR_INC = $(ROOT_DIR)/inc
CFLAGS = -g -Wall -I${DIR_INC}
 #子目录的Makefile直接读取其子目录就行
SUBDIRS=$(shell ls -l | grep ^d | awk '{print $$9}')
#以下同根目录下的makefile的相同代码的解释
CUR_SOURCE=${wildcard *.c}
CUR_OBJS=${patsubst %.c, %.o, $(CUR_SOURCE)}
all:$(SUBDIRS) $(CUR_OBJS)
$(SUBDIRS):ECHO
	make -C $@
$(CUR_OBJS):%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o  $(ROOT_DIR)/$(OBJS_DIR)/$@ $(FLAG)
ECHO:
	@echo $(SUBDIRS)
