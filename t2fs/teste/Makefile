CC=gcc
LIB_DIR=../lib
INC_DIR=../include
BIN_DIR=../bin
SRC_DIR=../src

all: $(LIB_DIR)/libt2fs.a ./teste.c
	gcc -o teste teste.c -L$(LIB_DIR) -lt2fs -Wall

clean:
	rm $(BIN_DIR)/t2fstst
