#
# Makefile ESQUELETO
#
# DEVE ter uma regra "all" para geração da biblioteca
# regra "clean" para remover todos os objetos gerados.
#
# NECESSARIO adaptar este esqueleto de makefile para suas necessidades.
#
#

CC=gcc
LIB_DIR=./lib
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src
TST_DIR=./teste
EXE_DIR=./exemplo

all: teste1 teste2


objetos: $(SRC_DIR)/t2fs.c $(INC_DIR)/apidisk.h $(INC_DIR)/t2fs.h
	$(CC) -c $(SRC_DIR)/t2fs.c

mvObj:
	mv *.o $(BIN_DIR)

lib.a: $(BIN_DIR)/t2fs.o
	ar crs libt2fs.a $(BIN_DIR)/*.o

mvLib:
	mv *.a $(LIB_DIR)

exemplo: $(EXE_DIR)/t2fstst.c
	$(CC) -o $(EXE_DIR)/t2fstst $(EXE_DIR)/t2fstst.c -L$(LIB_DIR) -I$(LIB_DIR)


clean:
	rm -rf $(LIB_DIR)/*.a $(BIN_DIR)/*.o $(SRC_DIR)/*~ $(INC_DIR)/*~ *~


teste1: $(SRC_DIR)/t2fs.c
	$(CC) -c $(SRC_DIR)/t2fs.c -I$(INC_DIR)/ -Wall -m32 -g
	mv t2fs.o $(BIN_DIR)/

teste2: $(BIN_DIR)/t2fs.o
	ar crs $(LIB_DIR)/libt2fs.a $(BIN_DIR)/t2fs.o $(LIB_DIR)/apidisk.o
