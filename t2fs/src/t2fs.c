#include "../include/apidisk.h"
#include "../include/t2fs.h"
#include <string.h>
#include <stdlib.h>

#define SUCESSO 0;
#define ERRO -1;
typedef struct t2fs_superbloco SB;
typedef struct t2fs_record RC;

int SectorsPerCluster;

SB SUPER;
RC *ROOT; // ROOT é um conjunto de records, ou seja, um diretorio. Aponta para primeiro record do diretorio
RC *CURRENT_DIR;

int CLUSTER_SIZE;
int RecsPerCluster;
int fscriado = 0;
char FILENAME[MAX_FILE_NAME_SIZE];
char PATH[MAX_FILE_NAME_SIZE]; // str auxiliar para percorrer nomes de arquivos



///////////////////////////////// Auxiliares

int read_cluster(int pos, *buffer){
  for(int i = 0; i < SectorsPerCluster; i++){
    if(!read_sector(pos+i*SECTOR_SIZE, buffer+i*SECTOR_SIZE)) return ERRO;
  }
  return SUCESSO;
};

int inicializa(){
  char buffer[SECTOR_SIZE];
  if(!read_sector(0, buffer)) return ERRO;
  memcpy(SUPER.id, buffer, size_t 4);
  memcpy(SUPER.version, buffer+4, size_t 2);
  memcpy(SUPER.SuperBlockSize, buffer+6, size_t 2);
  memcpy(SUPER.DiskSize, buffer+8, size_t 4);
  memcpy(SUPER.NofSectors, buffer+12, size_t 4);
  memcpy(SUPER.SectorsPerCluster, buffer+16, size_t 4);
  memcpy(SUPER.pFATSectorStart, buffer+20, size_t 4);
  memcpy(SUPER.RootDirCluster, buffer+24, size_t 4);
  memcpy(SUPER.DataSectorStart, buffer+28, size_t 4);

  SectorsPerCluster = SUPER.SectorsPerCluster;
  CLUSTER_SIZE = SECTOR_SIZE * SectorsPerCluster;
  RecsPerCluster = SectorsPerCluster * 4; // cabem 4 records por setor
  ROOT = (RC*) malloc(CLUSTER_SIZE);
  CURRENT_DIR = ROOT;
  if(!read_cluster(SUPER.RootDirCluster*SectorsPerCluster + SUPER.DataSectorStart, ROOT)) return ERRO;
  //memcpy(&ROOT + SUPER.DataSectorStart, SUPER+24, sizeof(ROOT));

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

RC* novoRC(RC* Dir_ptr){
  int i;
  for(i = 0; i<RecsPerCluster; i++){
    if(Dir_ptr .TypeVal == TYPEVAL_INVALIDO) return &();
  }
  return NULL;
}

///////////////////////////////////// Funções abaixo
int identify2 (char *name, int size){
  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }
  return ERRO;
}


FILE2 create2 (char *filename){
  int i;
  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }

  // se ponteiro filename eh null ou \0
  if(!filename) return ERRO;
  if(filename[0] != '/') return ERRO;

  int filenamesize = strlen(filename);
  // nome do arquivo eh maior que o permitido
  // MAX_FILE_NAME_SIZE definido no arquivo t2fs.h
  char *FILENAME = (char*) malloc(filenamesize);
  strcpy(FILENAME, filename);
  //if(filenamesize > MAX_FILE_NAME_SIZE) return ERRO;
  //strcpy(FILENAME, filename);

  tokens = str_split(FILENAME, '/');
  // localiza diretorio para criar
  /// corrigir para nao ler nome do arquivo a ser criado
  RC *tmpDir = ROOT;
  for(i=0; *(tokens + i); i++){
      tmpDir = get_RC_in_DIR(tmpDir, (tokens + i))
      if(tmpDir == NULL) return ERRO; // n existe subdiretorio com nome token atual em dir tmpDir
  }

  // recupera cluster de tmpDir mais filho na hierarquia
  tmpDir = &(tmpDir->firstCluster * SectorsPerCluster + SUPER.DataSectorStart);
  // acha entrada válida no diretório
  RC *arq = novoRC(tmpDir);
  strcpy(arq->name, nomearquivo);
  arq->bytesFileSize = CLUSTER_SIZE;
  arq->fistCluster = achaFat();
  //if(achaFat()) return ERRO; // nao ha mais CLUSTER livre para arquivo
  return SUCESSO;
}

*RC get_RC_in_DIR (RC* dir, *filename){
  int i;
  for(i = 0; i < RecsPerCluster; i++){
    if(strcmp( &((dir + i*sizeof(RC))->name), filename) == 0) return &(dir + i*sizeof(RC));
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
  int i;
  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }

  // se ponteiro filename eh null ou \0
  if(!filename) return ERRO;
  // abre arquivo em dir atual
  if(filename[0] != '/'){
    RC*  = (CURRENT_DIR);
  };
// else, cria em caminho absoluto especificado

  int filenamesize = strlen(filename);
  // nome do arquivo eh maior que o permitido
  // MAX_FILE_NAME_SIZE definido no arquivo t2fs.h
  char *FILENAME = (char*) malloc(filenamesize);
  strcpy(FILENAME, filename);
  //if(filenamesize > MAX_FILE_NAME_SIZE) return ERRO;
  //strcpy(FILENAME, filename);

  tokens = str_split(FILENAME, '/');
  // localiza diretorio para criar
  /// corrigir para nao ler nome do arquivo a ser criado
  RC *tmpDir = ROOT;
  for(i=0; *(tokens + i); i++){
      tmpDir = get_RC_in_DIR(tmpDir, (tokens + i))
      if(tmpDir == NULL) return ERRO; // n existe subdiretorio com nome token atual em dir tmpDir
  }

  // recupera cluster de tmpDir mais filho na hierarquia
  tmpDir = &(tmpDir->firstCluster * SectorsPerCluster + SUPER.DataSectorStart);
  // acha entrada válida no diretório
  RC *arq = acha_valido(tmpDir);
  strcpy(arq->name, nomearquivo);
  arq->bytesFileSize = CLUSTER_SIZE;
  arq->fistCluster = achaFat();
  //if(achaFat()) return ERRO; // nao ha mais CLUSTER livre para arquivo
  return SUCESSO;

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

/// aqui vai o handle?
  RC *record = get_RC_in_DIR();
  if(record != NULL) {
    strcpy(&(dentry->name), &(records->name));
    dentry->fileType = record->TypeVal;
    dentry->fileSize = record->bytesFileSize;
    return SUCESSO;
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
