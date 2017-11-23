#include "../include/apidisk.h"
#include "../include/t2fs.h"
#include <string.h>
#include <stdlib.h>

#define SUCESSO 0;
#define ERRO -1;

struct t2fs_superbloco SUPER;

int fscriado = 0;
char[55] filename;

int inicializa(){
  //FILE *disco = fopen("../t2fs.dat", "rw");

  char[SECTOR_SIZE] buffer;
  read_sector (0, buffer);
  /*

  Creio que tenhamos que ler as infos abaixo do t2fs.dat
  SUPER.id = buffer[];
  SUPER.version
  SUPER.SuperBlockSize
  SUPER.DiskSize
  SUPER.NofSectors
  SUPER.SectorsPerCluster
  SUPER.pFATSectorStart
  SUPER.RootDirCluster
  SUPER.DataSectorStart
  */

  memcpy(SUPER.id, buffer, size_t 4);
  memcpy(SUPER.version, buffer+4, size_t 2);
  memcpy(SUPER.SuperBlockSize, buffer+6, size_t 2);
  memcpy(SUPER.DiskSize, buffer+8, size_t 4);
  memcpy(SUPER.NofSectors, buffer+12, size_t 4);
  memcpy(SUPER.SectorsPerCluster, buffer+16, size_t 4);
  memcpy(SUPER.pFATSectorStart, buffer+20, size_t 4);
  memcpy(SUPER.RootDirCluster, buffer+24, size_t 4);
  memcpy(SUPER.DataSectorStart, buffer+28, size_t 4);
  return ERRO;
};

int identify2 (char *name, int size){
  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }
  return ERRO;
}

FILE2 create2 (char *filename){
  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }

  // se ponteiro filename eh null
  if(!filename) return ERRO;
  int filenamesize = strlen(filename.name);

  // nome do arquivo eh maior que o permitido
  // MAX_FILE_NAME_SIZE definido no arquivo t2fs.h
  if(filenamesize > MAX_FILE_NAME_SIZE) return ERRO;

  // ve se nome absoluto ou relativo
  // localiza entrada se absoluto, salvando a localização atual, para voltar depois


  return ERRO;
}

int delete2 (char *filename){
  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }
  return ERRO;
}

FILE2 open2 (char *filename){
  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }

  // ve se nome absoluto ou relativo
  // localiza entrada se absoluto, salvando a localização atual, para voltar depois
  // ve se arquivo existe no diretorio especificado


  return ERRO;
}

int close2 (FILE2 handle) {
  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }
  return ERRO;
}

int read2 (FILE2 handle, char *buffer, int size){
  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }
  return ERRO;
}

int write2 (FILE2 handle, char *buffer, int size){
  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }
  return ERRO;
}

int truncate2 (FILE2 handle){
  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }
  return ERRO;
}

int seek2 (FILE2 handle, unsigned int offset){
  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }
  return ERRO;
}

int mkdir2 (char *pathname){
  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }
  return ERRO;
}

int rmdir2 (char *pathname){
  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }
  return ERRO;
}

int chdir2 (char *pathname){
  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }
  return ERRO;
}

int getcwd2 (char *pathname, int size){
  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }
  return ERRO;
}

DIR2 opendir2 (char *pathname){
  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }
  return ERRO;
}

int readdir2 (DIR2 handle, DIRENT2 *dentry){
  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }
  return ERRO;
}

int closedir2 (DIR2 handle){
  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }
  return ERRO;
}
