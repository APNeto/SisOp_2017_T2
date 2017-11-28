#include "../include/apidisk.h"
#include "../include/t2fs.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define SUCESSO 0
#define ERRO -1
typedef struct t2fs_superbloco SB;
typedef struct t2fs_record RC;

int SectorsPerCluster;

SB SUPER;
RC *ROOT; // ROOT é um conjunto de records, ou seja, um diretorio. Aponta para primeiro record do diretorio
RC *CURRENT_DIR;
DWORD *FAT;
RC *BUFF;

int FATstart;
int FATtotalSize;
int num_dir_open = 0;
int num_file_open = 0;
int CLUSTER_SIZE;
int RecsPerCluster;
int fscriado = 0;
char FILENAME[MAX_FILE_NAME_SIZE];
char PATH[MAX_FILE_NAME_SIZE]; // str auxiliar para percorrer nomes de arquivos


struct lista{
  int current_pointer;
  int handle;
  int pos; // usaremos a posicao do arquivo em disco para sabermos se ele esta aberto ou nao
  struct lista *prox;
  struct lista *ant;
};

struct lista *open_files_list;
struct lista *open_dir_list;



///////////////////////////////// Auxiliares

int read_cluster(int pos, char *buffer){
  int i;
  for(i = 0; i < SectorsPerCluster; i++){
    if(!read_sector(pos+i*SECTOR_SIZE, (char*) &buffer[i*SECTOR_SIZE])) return ERRO;
  }
  return SUCESSO;
};

int inicializa(){
  int i;
  char buffer[SECTOR_SIZE];
  if(!read_sector(0, buffer)) return ERRO;
  memcpy(&SUPER.id, buffer, 4);
  memcpy(&SUPER.version, buffer+4, 2);
  memcpy(&SUPER.SuperBlockSize, buffer+6, 2);
  memcpy(&SUPER.DiskSize, buffer+8, 4);
  memcpy(&SUPER.NofSectors, buffer+12, 4);
  memcpy(&SUPER.SectorsPerCluster, buffer+16, 4);
  memcpy(&SUPER.pFATSectorStart, buffer+20, 4);
  memcpy(&SUPER.RootDirCluster, buffer+24, 4);
  memcpy(&SUPER.DataSectorStart, buffer+28, 4);

  SectorsPerCluster = SUPER.SectorsPerCluster;
  CLUSTER_SIZE = SECTOR_SIZE * SectorsPerCluster;
  RecsPerCluster = SectorsPerCluster * 4; // cabem 4 records por setor ou SECTOR_SIZE/sizeof(RC)
  ROOT = (RC*) malloc(CLUSTER_SIZE);
  CURRENT_DIR = ROOT;
  if(!read_cluster(SUPER.RootDirCluster*SectorsPerCluster*SECTOR_SIZE + SUPER.DataSectorStart, (char*) ROOT)) return ERRO;
  //memcpy(&ROOT + SUPER.DataSectorStart, SUPER+24, sizeof(ROOT));
  BUFF  = (RC*) malloc(CLUSTER_SIZE);
  FATtotalSize = SECTOR_SIZE * (SUPER.DataSectorStart - SUPER.pFATSectorStart);
  FAT = (DWORD*) malloc(FATtotalSize);
  for(i=0; i<FATtotalSize; i+=SECTOR_SIZE){
    if(read_sector(SUPER.pFATSectorStart+i, (char*) &FAT[i]) != 0) return ERRO;
  }
};

int alocateCluster(){
  int i;
  for(i = 0; i < FATtotalSize; i++){
    if( *(FAT+i) == 0) return i;
  }
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
  read_cluster(Dir_ptr->firstCluster*SectorsPerCluster*SECTOR_SIZE+SUPER.DataSectorStart, (char*) BUFF);
  for(i = 0; i<RecsPerCluster; i++){
    if(BUFF[i].TypeVal == TYPEVAL_INVALIDO) return &BUFF[i];
  }
  return NULL;
}

RC* get_RC_in_DIR (RC* dir, char *filename){
  int i;
  //BUFF = (RC*) malloc(CLUSTER_SIZE);
  //read_cluster(dir->firstCluster*SectorsPerCluster*SECTOR_SIZE+SUPER.DataSectorStart, BUFF);
  for(i = 0; i < RecsPerCluster; i++){
    if(strcmp( dir[i].name, filename) == 0) return &(dir[i]);
  }
  return NULL;
}
RC* get_next_dir(RC* dir, char *filename){
  RC* tmp;
  //BUFF = (RC*) malloc(CLUSTER_SIZE);
  tmp = get_RC_in_DIR(dir, filename);
  if(tmp == NULL || tmp->TypeVal != TYPEVAL_DIRETORIO) return NULL;
  read_cluster(tmp->firstCluster*CLUSTER_SIZE+SUPER.DataSectorStart, (char*) BUFF);
  //tmp = (RC*) (tmp->firstCluster * CLUSTER_SIZE + SUPER.DataSectorStart);
  //return tmp;
  return BUFF;
}

int resetFAT(int numFat){
  int i;
  FAT[numFat] = 0x00000000;
  for(i=0; i<FATtotalSize; i+=SECTOR_SIZE)
    if(write_sector(FATstart + i, (char*) &(FAT[i])) != 0) return ERRO;

  return SUCESSO;
}

int apagaFatArq(RC* arq){
  int ind = arq->firstCluster;
  int numCluster = arq->bytesFileSize / CLUSTER_SIZE;
  if((arq->bytesFileSize % CLUSTER_SIZE) > 0.0) numCluster++;
  int i, tmp;
  for(i=0; i<numCluster; i++){
    tmp = FAT[ind];
    FAT[ind] = 0x00000000;
    ind = tmp;
  }
  return SUCESSO;
}

int achaFat() {
  int i;
  for(i=0; i<FATtotalSize;i++){
    if(FAT[i] == 0x00000000) return i;
  }
  return ERRO;
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
  char **tokens;
  RC *buffer;
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
  for(i=0; *(tokens + i+2); i++){
      tmpDir = get_next_dir(tmpDir, *(tokens + i));
      if(tmpDir == NULL) return ERRO; // n existe subdiretorio com nome token atual em dir tmpDir
  }

  // recupera cluster de tmpDir mais filho na hierarquia
  //buffer = malloc(CLUSTER_SIZE);
  //read_cluster(tmpDir->firstCluster * SectorsPerCluster + SUPER.DataSectorStart, buffer);
  // acha entrada válida no diretório
  RC *arq = novoRC(tmpDir);
  int fatNum = achaFat();
  if(fatNum == ERRO) return ERRO;
  arq->firstCluster = fatNum;
  if(strlen(*(tokens+i)) > 54) return ERRO;
  strcpy(&(arq->name), *(tokens+i+1));
  arq->bytesFileSize = CLUSTER_SIZE;
  arq->TypeVal = TYPEVAL_REGULAR;
  //if(achaFat()) return ERRO; // nao ha mais CLUSTER livre para arquivo
  return SUCESSO;
}


int delete2 (char *filename){
  int i;
  char **tokens;
  RC *buffer;
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
  // localiza diretorio para apagar
  RC *tmpDir = ROOT;
  for(i=0; *(tokens + i+2); i++){
      tmpDir = get_next_dir(tmpDir, *(tokens + i));
      if(tmpDir == NULL) return ERRO; // n existe subdiretorio com nome token atual em dir tmpDir
  }

  RC *arq = get_RC_in_DIR(tmpDir, *(tokens+i+1) );
  if(arq == NULL || arq->TypeVal) return ERRO;
  apagaFat(arq);
  //arq->bytesFileSize;
  arq->TypeVal = TYPEVAL_INVALIDO;
  //if(achaFat()) return ERRO; // nao ha mais CLUSTER livre para arquivo
  return SUCESSO;
}

/*
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
    for(i=0; i<RecsPerCluster; i++){
      int res = strcmp(CURRENT_DIR+i*sizeof(RC))->name, filename);
      if(res == 0) break;
    }
    if(res != 0) return ERRO;
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

  num_file_open++;
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
  if(handle < 0) return ERRO;
  // recupera handle
  num_file_open--;
  return ERRO;
}

int read2 (FILE2 handle, char *buffer, int size){
  int i,j;
  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }

  // recupera estrutura junto ao handle
  // recupera cluster atual do arquivo?
  /*for(i=0, j=0; i<size;i++, j++){
    buffer[i] = *(current_pointer+j);
    current_pointer += j;
    if(){ // chegou ao fim do cluster atual
      j = 0;
      // recupera proximo cluster
      current_pointer =  ;
    };
  }
  *
  return i; // numero de bytes lidos

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
  char **tokens;
  RC *tmpDir, *tmpPai;
  int i, j;
  int numCluster;
  char buffer[CLUSTER_SIZE];
  char *FILENAME = (char*) malloc(strlen(pathname));

  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }

  if(pathname == NULL || pathname[0] == '\0') return ERRO;
  if(pathname[0] == '/'){ // caminho absoluto
    tmpDir = ROOT;
  }
  else{ // caminho relativo
    tmpDir = CURRENT_DIR;
  }
  strcpy(FILENAME, pathname);
  tokens = str_split(FILENAME, '/');


  // aqui há um +1 para criterio de parada parar no diretorio pai
  for(i = 0; *(tokens + i + 1); i++){
    paiDir = get_next_dir(tmpDir, *(tokens + i));
    // pathname nao existe
    if(paiDir == NULL || tmpDir->TypeVal != TYPEVAL_DIRETORIO) return ERRO;
  }

  // acha entrada nao ocupada no diretorio pai
  for(j = 0; j< RecsPerCluster; j++){
    if( (tmpDir+j*64)->TypeVal == TYPEVAL_INVALIDO) break;
  }

  // se for acima foi todo percorrido sem achar entrada desocupada
  if( (tmpDir+j*64)->TypeVal != TYPEVAL_INVALIDO) return ERRO;
  // senao
  numCluster = alocateCluster();
  if(numCluster < 0) return ERRO;
  (tmpDir+j*64)->TypeVal = TYPEVAL_DIRETORIO;
  strcpy( &((tmpDir+j*64)->name), *(tokens + i) );
  (tmpDir+j*64)->bytesFileSize = CLUSTER_SIZE;
  (tmpDir+j*64)->firstCluster = numCluster;

  // salva posicao da FAT como ultimo cluster do arquivo (diretorio)
  FAT[numCluster] = 0xFFFFFFFF;

  /// zerar todas entradas do diretorio para ter certeza que está conforme, para quando alguma entreda for usada
  return SUCESSO;
}

int rmdir2 (char *pathname){
  char **tokens;
  RC *tmpDir, *paiDir, *dirRC;
  int i;
  char buffer[CLUSTER_SIZE];
  char *FILENAME = (char*) malloc(strlen(pathname));

  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }

  if(pathname == NULL || pathname[0] == '\0') return ERRO;
  if(pathname[0] == '/'){ // caminho absoluto
    tmpDir = ROOT;
  }
  else{ // caminho relativo
    tmpDir = CURRENT_DIR;
  }
  strcpy(FILENAME, pathname);
  tokens = str_split(FILENAME, '/');

  paiDir = tmpDir;
  i = 0;
  // aqui há um +1 para criterio de parada parar no diretorio pai
  for(; *(tokens + i + 1); i++){
    paiDir = get_next_dir(paiDir, *(tokens + i));
    // pathname nao existe
    if(paiDir == NULL) return ERRO;
  }
  // pega dir de ultima string da sequencia do pathname
  tmpDir = get_next_dir(paiDir, *(tokens + i));

  // se diretorio nao vazio
  for(i = 0; i< RecsPerCluster; i++){
    if( (tmpDir+i*64)->TypeVal != TYPEVAL_INVALIDO) return ERRO;
  }

  dirRC = get_RC_in_DIR(paiDir, *(tokens + i));
  dirRC->firstCluster;




  return ERRO;
}

int chdir2 (char *pathname){
  int i;
  char *PATHNAME;

  if(pathname == NULL || pathname[0] == '\0') return ERRO;

  PATHNAME = (char *) malloc(strlen(pathname));

  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }

  if(pathname[0] == '/'){ // caminho absoluto
    tmpDir = ROOT;
  }
  else{ // caminho relativo
    tmpDir = CURRENT_DIR;
  }

  strcpy(PATHNAME, pathname);
  tokens = str_split(PATHNAME, '/');

  // aqui há um +1 para criterio de parada parar no diretorio pai
  for(i = 0; *(tokens + i ); i++){
    tmpDir = get_next_dir(tmpDir, *(tokens + i));
    // pathname nao existe
    if(tmpDir == NULL) return ERRO;
  }

  return SUCESSO;
}

int getcwd2 (char *pathname, int size){
  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }
  return ERRO;
}

DIR2 opendir2 (char *pathname){
  int handle = 0;
  RC *tmpDir;
  int i;
  char buffer[CLUSTER_SIZE];
  char *FILENAME = (char*) malloc(strlen(pathname));

  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }

  if(pathname == NULL || pathname[0] == '\0') return ERRO;
  if(pathname[0] == '/'){ // caminho absoluto
    tmpDir = ROOT;
  }
  else{ // caminho relativo
    tmpDir = CURRENT_DIR;
  }
  strcpy(FILENAME, pathname);
  tokens = str_split(FILENAME, '/');

  for(i = 0; *(tokens + i); i++){
    tmpDir = get_next_dir(tmpDir, *(tokens + i));
    if(tmpDir == NULL) return ERRO;
  }

  // colocar handle em lista de diretorios abertos
  num_dir_open++;
  return handle;
  return ERRO;
}

int readdir2 (DIR2 handle, DIRENT2 *dentry){
  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }
  if( 0 > handle || handle > open_dir_open) return ERRO;
  if(dentry == NULL) return ERRO;
  /// recuperar Record de handle
  /// aqui vai o handle?
  RC *record = get_RC_in_DIR();
  if(record != NULL) {
    strcpy(&(dentry->name), &(record->name));
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

  if(handle < 0) return ERRO;
  // recuperar RC de handle?
  // handle em lista de diretorios abertos

  num_dir_open--;
  return SUCESSO;
  return ERRO;
}
*/
