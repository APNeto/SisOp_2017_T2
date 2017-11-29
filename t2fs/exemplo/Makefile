CC=gcc
LIB_DIR=../lib
INC_DIR=../include
BIN_DIR=../bin
SRC_DIR=../src

all:
	gcc -c $(SRC_DIR)/t2fs.c -o $(LIB_DIR)/t2fs.o -Wall
	ar crs $(LIB_DIR)/libt2fs.a $(LIB_DIR)/t2fs.o $(LIB_DIR)/apidisk.o
	gcc -o $(BIN_DIR)/t2fstst t2fstst.c -L$(LIB_DIR) -lt2fs -Wall

clean:
	rm $(BIN_DIR)/t2fstst
