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

all: $(LIB_DIR)/t2fs

t2fs: $(LIB_DIR)/t2fs.o
	$(CC) -o $@ $< $(LIB_DIR)/apidisk.o -I$(INC_DIR) 

$(LIB_DIR)/t2fs.o: $(SRC_DIR)/t2fs.c
	$(CC) -c -o $@ $< -I$(INC_DIR)

clean:
	rm -rf $(LIB_DIR)/*.a $(BIN_DIR)/*.o $(SRC_DIR)/*~ $(INC_DIR)/*~ *~


