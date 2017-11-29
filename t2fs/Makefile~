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

all: objetos mvObj lib.a mvLib exemplo


objetos: $(SRC_DIR)/t2fs.c $(INC_DIR)/apidisk.h $(INC_DIR)/t2fs.h 
	$(CC) -c $(SRC_DIR)/t2fs.c

mvObj: 
	mv *.o $(BIN_DIR)

lib.a: $(BIN_DIR)/t2fs.o
	ar crs libt2fs.a $(BIN_DIR)/*.o

mvLib:
	mv *.a $(LIB_DIR)

exemplo: 
	$(CC) -o  $(EXE_DIR)/t2fstst $(EXE_DIR)/t2fstst.c -L$(LIB_DIR) -libt2fs


clean:
	rm -rf $(LIB_DIR)/*.a $(BIN_DIR)/*.o $(SRC_DIR)/*~ $(INC_DIR)/*~ *~

