CC = mipsel-openwrt-linux-gcc
FLAG = -lfcgi -ljson-c -lssl -lcrypto -lblkid
# -ludev -levdev

DIR_INC = $(ROOT_DIR)/inc
CFLAGS = -g -Wall -I${DIR_INC}

BIN_TARGET = ${ROOT_DIR}/${BIN_DIR}/${BIN}
OBJS=*.o
ODIR=objs

${BIN_TARGET}:$(ODIR)/$(OBJS)
	$(CC) -o $@ $^ $(FLAG)
