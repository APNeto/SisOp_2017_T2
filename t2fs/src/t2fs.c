#include "../include/apidisk.h"
#include "../include/t2fs.h"
#include <string.h>
#include <stdlib.h>

#define SUCESSO 0;
#define ERRO -1;
typedef struct t2fs_superbloco SB;
typedef struct t2fs_record RT;

int SectorsPerCluster;

SB SUPER[SECTOR_SIZE];

int fscriado = 0;
char FILENAME[MAX_FILE_NAME_SIZE];
char PATH[MAX_FILE_NAME_SIZE];



///////////////////////////////// Auxiliares

int read_cluster(int pos, *buffer){
  for(int i = 0; i < SectorsPerCluster; i++){
    if(!read_sector(pos+i*SECTOR_SIZE, buffer+i*SECTOR_SIZE)) return ERRO;
  }
  return SUCESSO;
};

int inicializa(){
  if(!read_sector(0, SUPER)) return ERRO;

  SectorsPerCluster = *(SUPER+16);
  int root_local = SUPER+24;
  memcpy(&ROOT, SUPER+24, sizeof(ROOT));

  return ERRO;
};


char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

///////////////////////////////////// Funções abaixo
int identify2 (char *name, int size){
  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }
  return ERRO;
}


_
FILE2 create2 (char *filename){
  int i;
  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }

  // se ponteiro filename eh null ou \0
  if(!filename) return ERRO;
  if(filename[0] != '\\') return ERRO;

  int filenamesize = strlen(filename.name);
  // nome do arquivo eh maior que o permitido
  // MAX_FILE_NAME_SIZE definido no arquivo t2fs.h
  if(filenamesize > MAX_FILE_NAME_SIZE) return ERRO;
  strcpy(FILENAME, filename);

  tokens = str_split(FILENAME, '\\');
  // localiza diretorio para criar
  // corrigir para nao ler nome do arquivo a ser criado
  for(i=0; *(tokens + i); i++){
      *(tokens + i);
  }

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
