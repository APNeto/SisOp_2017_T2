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
char *CURRPATH;

struct lista{
  int usado;
  char name[MAX_FILE_NAME_SIZE];
  DWORD firstCluster;
  int current_pointer;
};

//struct lista *open_files_list;
//struct lista *open_dir_list;

struct lista open_dir[10] = {0};
struct lista open_files[10] = {0};

///////////////////////////////// Auxiliares

int read_cluster(int pos, char *buffer){
  int i;
  for(i = 0; i < SectorsPerCluster; i++){
    if(!read_sector(pos+i*SECTOR_SIZE+SUPER.DataSectorStart, (char*) &buffer[i*SECTOR_SIZE])) return ERRO;
  }
  return SUCESSO;
};

int write_cluster(int pos, (RC*) buffer ){
  int i;
  for(i = 0; i <CLUSTER_SIZE; i+=SectorsPerCluster)
    if(write_sector( pos+i, (char*) buffer+i) != 0) return ERRO;
  return SUCESSO;
}


int inicializa(){
  int i;
  char buffer[SECTOR_SIZE];
  if(!read_sector(0, buffer)) return ERRO;
  strncpy(SUPER.id, buffer,4);
  SUPER.version = *((WORD *)(buffer + 4));
  SUPER.SuperBlockSize = *((WORD *)(buffer + 6));
  SUPER.DiskSize = *((DWORD *)(buffer + 8));
  SUPER.NofSectors = *((DWORD *)(buffer + 12));
  SUPER.SectorsPerCluster = *((DWORD *)(buffer + 16));
  SUPER.pFATSectorStart = *((DWORD *)(buffer + 20));
  SUPER.RootDirCluster = *((DWORD *)(buffer + 24));
  SUPER.DataSectorStart = *((DWORD *)(buffer + 28));

  SectorsPerCluster = SUPER.SectorsPerCluster;
  CLUSTER_SIZE = SECTOR_SIZE * SectorsPerCluster;
  RecsPerCluster = SectorsPerCluster * 4; // cabem 4 records por setor ou SECTOR_SIZE/sizeof(RC)
  ROOT = (RC*) malloc(CLUSTER_SIZE);
  CURRENT_DIR = (RC*) malloc(CLUSTER_SIZE);
  if(!read_cluster(SUPER.RootDirCluster*SectorsPerCluster*SECTOR_SIZE + SUPER.DataSectorStart, (char*) ROOT)) return ERRO;
  if(!read_cluster(SUPER.RootDirCluster*SectorsPerCluster*SECTOR_SIZE + SUPER.DataSectorStart, (char*) CURRENT_DIR)) return ERRO;
  //memcpy(&ROOT + SUPER.DataSectorStart, SUPER+24, sizeof(ROOT));
  BUFF  = (RC*) malloc(CLUSTER_SIZE);
  FATtotalSize = SECTOR_SIZE * (SUPER.DataSectorStart - SUPER.pFATSectorStart);
  FAT = (DWORD*) malloc(FATtotalSize);
  int j;
  for(j=0;j<10;j++){
  open_dir[j].current_pointer = -1;
  open_files[j].current_pointer= -1;
  }
  for(i=0; i<FATtotalSize; i+=SECTOR_SIZE){
    if(read_sector(SUPER.pFATSectorStart+i, (char*) &FAT[i]) != 0) return ERRO;
  }
  return SUCESSO;
}

int alocateCluster(){
  int i;
  for(i = 0; i < FATtotalSize; i++){
    if( *(FAT+i) == 0) return i;
  }
  return ERRO;
};

int str_split (const char *str, char c, char ***arr)
{
    int count = 1;
    int token_len = 1;
    int i = 0;
    char *p;
    char *t;

    p = str;
    while (*p != '\0')
    {
        if (*p == c)
            count++;
        p++;
    }

    *arr = (char**) malloc(sizeof(char*) * count);
    if (*arr == NULL)
        exit(1);

    p = str;
    while (*p != '\0')
    {
        if (*p == c)
        {
            (*arr)[i] = (char*) malloc( sizeof(char) * token_len );
            if ((*arr)[i] == NULL)
                exit(1);

            token_len = 0;
            i++;
        }
        p++;
        token_len++;
    }
    (*arr)[i] = (char*) malloc( sizeof(char) * token_len );
    if ((*arr)[i] == NULL)
        exit(1);

    i = 0;
    p = str;
    t = ((*arr)[i]);
    while (*p != '\0')
    {
        if (*p != c && *p != '\0')
        {
            *t = *p;
            t++;
        }
        else
        {
            *t = '\0';
            i++;
            t = ((*arr)[i]);
        }
        p++;
    }

    return count;
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

RC* get_next_dir2(RC* dir, char *filename, RC* dest){
  RC* tmp;
  //BUFF = (RC*) malloc(CLUSTER_SIZE);
  tmp = get_RC_in_DIR(dir, filename);
  if(tmp == NULL || tmp->TypeVal != TYPEVAL_DIRETORIO) return NULL;
  read_cluster(tmp->firstCluster*CLUSTER_SIZE+SUPER.DataSectorStart, (char*) dest);
  //tmp = (RC*) (tmp->firstCluster * CLUSTER_SIZE + SUPER.DataSectorStart);
  //return tmp;
  return dest;
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

int achaFatArq() {
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

  int tst = str_split(FILENAME, '/', &tokens);
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
  int fatNum = achaFatArq();
  if(fatNum == ERRO) return ERRO;
  arq->firstCluster = fatNum;
  if(strlen(*(tokens+i)) > 54) return ERRO;
  strcpy((arq->name), *(tokens+i+1));
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

  int tst = str_split(FILENAME, '/', &tokens);
  // localiza diretorio para apagar
  RC *tmpDir = ROOT;
  for(i=0; *(tokens + i+2); i++){
      tmpDir = get_next_dir(tmpDir, *(tokens + i));
      if(tmpDir == NULL) return ERRO; // n existe subdiretorio com nome token atual em dir tmpDir
  }

  RC *arq = get_RC_in_DIR(tmpDir, *(tokens+i+1) );
  if(arq == NULL || arq->TypeVal) return ERRO;
  apagaFatArq(arq);
  //arq->bytesFileSize;
  arq->TypeVal = TYPEVAL_INVALIDO;
  //if(achaFat()) return ERRO; // nao ha mais CLUSTER livre para arquivo
  return SUCESSO;
}


FILE2 open2 (char *filename){
  int i;
  int res;
  char **tokens;
  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }

  // se ponteiro filename eh null ou \0
  if(!filename) return ERRO;
  // abre arquivo em dir atual
  if(filename[0] != '/'){
    for(i=0; i<RecsPerCluster; i++){
      res = strcmp((CURRENT_DIR+i*sizeof(RC))->name, filename);
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

  int tst = str_split(FILENAME, '/', &tokens);
  // localiza diretorio para criar
  /// corrigir para nao ler nome do arquivo a ser criado
  RC *tmpDir = ROOT;
  for(i=0; *(tokens + i+1); i++){
      tmpDir = get_RC_in_DIR(tmpDir, *(tokens + i));
      if(tmpDir == NULL) return ERRO; // n existe subdiretorio com nome token atual em dir tmpDir
  }

  // recupera cluster de tmpDir mais filho na hierarquia

  //tmpDir = (tmpDir->firstCluster * CLUSTER_SIZE + SUPER.DataSectorStart);

  // acha entrada válida no diretório
  RC *arq = novoRC(tmpDir);
  strcpy(arq->name, *(tokens + i));
  arq->bytesFileSize = CLUSTER_SIZE;
  arq->firstCluster = achaFatArq();
  //if(achaFat()) return ERRO; // nao ha mais CLUSTER livre para arquivo

  int j=0;
  while(open_files[j].current_pointer!=-1){
   j++;
   if(j>10)break;
  }
  if(j>=10){
  //printf("Já existem 10 diretorios abertos");
  return ERRO;
}
  //strncpy(open_files[j].name,arq->name,56);
  //open_files[j].firstCluster = arq->fistCluster;
//  open_files[j].current_pointer=0;

// colocar handle em lista de diretorios abertos
  //num_dir_open++;
  //return handle;
  return j;
  //num_file_open++;
  //return SUCESSO;

  // localiza entrada se absoluto, salvando a localização atual, para voltar depois
  // ve se arquivo existe no diretorio especificado
}


int close2 (FILE2 handle) {
   if(!fscriado) {
    inicializa();
    fscriado = 1;
  }
  if(open_files[handle].current_pointer > 0)
  {
    open_files[handle].current_pointer=-1;
    return 0;
  }
  return ERRO;
}

/*
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
*/
int mkdir2 (char *pathname){
  char **tokens;
  RC *tmpDir, *paiDir;
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
  int tst = str_split(FILENAME, '/', &tokens);

  paiDir = tmpDir;
  // aqui há um -1 para criterio de parada parar no diretorio pai
  for(i = 0; i  < tst-1; i++){
    paiDir = get_next_dir(paiDir, tokens[i]);
    // pathname nao existe
    if(paiDir == NULL || paiDir->TypeVal != TYPEVAL_DIRETORIO) return ERRO;
    //if(i + 2 == 0) numClusterPai = get_RC_in_DIR(paiDir, *(tokens + i + 1))->firstCluster;
  }

  // acha entrada nao ocupada no diretorio pai
  for(j = 0; j< RecsPerCluster; j++){
    if( paiDir[j].TypeVal == TYPEVAL_INVALIDO) break;
  }
  if(j == RecsPerCluster) return ERRO;

  // se for acima foi todo percorrido sem achar entrada desocupada
  //if( tmpDir[j].TypeVal != TYPEVAL_INVALIDO) return ERRO;
  // senao
  numCluster = alocateCluster();
  if(numCluster < 0) return ERRO;
  paiDir[j].TypeVal = TYPEVAL_DIRETORIO;
  strcpy( paiDir[j].name, tokens[i] );
  paiDir[j].bytesFileSize = CLUSTER_SIZE;
  paiDir[j].firstCluster = numCluster;

  tmpDir = get_RC_in_DIR(paiDir, ".");
  write_cluster( SUPER.DataSectorStart+tmpDir->firstCluster*CLUSTER_SIZE,BUFF);
  // salva posicao da FAT como ultimo cluster do arquivo (diretorio)
  FAT[numCluster] = 0xFFFFFFFF;
  for(i=0; i<FATtotalSize; i+=SECTOR_SIZE)
    if(write_sector(FATstart + i, (char*) &(FAT[i])) != 0) return ERRO;
  /// zerar todas entradas do diretorio para ter certeza que está conforme, para quando alguma entreda for usada
  return SUCESSO;
}


int rmdir2 (char *pathname){
  char **tokens;
  RC *tmpDir, *paiDir, *dirRC;
  int i;
  char buffer[CLUSTER_SIZE];
  char *FILENAME = (char*) malloc(strlen(pathname));
  int numClusterPai;

  tmpDir = (RC*) malloc(CLUSTER_SIZE);

  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }

  if(pathname == NULL || pathname[0] == '\0') return ERRO;
  if(pathname[0] == '/'){ // caminho absoluto
    paiDir = ROOT;
  }
  else{ // caminho relativo
    paiDir = CURRENT_DIR;
  }
  strcpy(FILENAME, pathname);
  int tst = str_split(FILENAME, '/', &tokens);

  i = 0;
  // aqui há um +1 para criterio de parada parar no diretorio pai
  for(; *(tokens + i + 1); i++){
    paiDir = get_next_dir(paiDir, *(tokens + i));
    // pathname nao existe
    if(paiDir == NULL) return ERRO;
    if(*(tokens + i + 2) == 0) numClusterPai = get_RC_in_DIR(paiDir, *(tokens + i + 1))->firstCluster;
  }
  // pega dir de ultima string da sequencia do pathname
  tmpDir = get_next_dir2(paiDir, *(tokens + i), tmpDir);

  // se diretorio nao vazio
  int j;
  for(j = 0; i< RecsPerCluster; j++){
    if( tmpDir[j].TypeVal != TYPEVAL_INVALIDO) return ERRO;
  }
  free(tmpDir);
  dirRC = get_RC_in_DIR(paiDir, *(tokens + i+1));
  resetFAT(dirRC->firstCluster);
  dirRC->TypeVal = TYPEVAL_INVALIDO;
  // Falta escrever o cluster do pai de volta no disco
  write_cluster( SUPER.DataSectorStart+numClusterPai*CLUSTER_SIZE,BUFF);
  return SUCESSO;
}
/*
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
    tmpDir = CURRPATH;
  }

  strcpy(PATHNAME, pathname);
  tokens = str_split(PATHNAME, '/');

  // aqui há um +1 para criterio de parada parar no diretorio pai
  for(i = 0; *(tokens + i +1); i++){
    tmpDir = get_next_dir(tmpDir, *(tokens + i));
    // pathname nao existe
    if(tmpDir == NULL) return ERRO;
  }
  RC* arq = get_next_RC(tmpDir, *(tokens+i+1));
  if(arq == NULL) return ERRO;
  read_cluster(arq->firstCluster*CLUSTER_SIZE+SUPER.DataSectorStart, (char*) CURRENT_DIR);
  // se path absoluto, soh copiar para o currpath if()
  CURRPATH = (char*) malloc(strlen(pathname);

  return SUCESSO;
}

int getcwd2 (char *pathname, int size){
  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }
  strcpy(pathname, CURRPATH);
  return ERRO;
}

*/

DIR2 opendir2 (char *pathname){
  int handle = 0;
  RC *tmpDir;
  char **tokens;
  int i;
  char buffer[CLUSTER_SIZE];
  if(pathname == NULL || pathname[0] == '\0') return ERRO;
  if(pathname[0] = '/') return SUCESSO;
  char *FILENAME = (char*) malloc(sizeof(char)*strlen(pathname));

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
  int tst = str_split(FILENAME, '/', &tokens);

  for(i = 0; i < teste; i++){
    tmpDir = get_next_dir(tmpDir, *(tokens + i));
    if(tmpDir == NULL) return ERRO;
  }
  int j=0;
  while(open_dir[j].current_pointer!=-1){
   j++;
   if(j==10) return ERRO;//printf("Já existem 10 diretorios abertos");
  }

  strncpy(open_dir[j].name,tmpDir->name, MAX_FILE_NAME_SIZE);
  open_dir[j].firstCluster = tmpDir->firstCluster;
  open_dir[j].current_pointer=0;
  // colocar handle em lista de diretorios abertos
  //num_dir_open++;
  //return handle;
  return j;
}


int readdir2 (DIR2 handle, DIRENT2 *dentry){
  int i;
  RC *record;
  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }
  if( 0 > handle || handle > 10 - 1) return ERRO;
  if(dentry == NULL) return ERRO;
  /// recuperar Record de handle
  /// aqui vai o handle?
  if(open_dir[handle].usado == 0) return ERRO;

  read_cluster(open_dir[handle].firstCluster*CLUSTER_SIZE+SUPER.DataSectorStart, (char*) BUFF);

  for(i=open_dir[handle].current_pointer; i<RecsPerCluster; i+=sizeof(RC)){
    if(BUFF[i].TypeVal != TYPEVAL_INVALIDO) {
    strcpy( dentry->name, BUFF[i].name);
    dentry->fileType = BUFF[i].TypeVal;
    dentry->fileSize = BUFF[i].bytesFileSize;
    return SUCESSO;
    }
  }
  return ERRO;
}


int closedir2 (DIR2 handle){
  if(!fscriado) {
    inicializa();
    fscriado = 1;
  }
  if(open_dir[handle].current_pointer>0)
  {
    open_dir[handle].current_pointer=-1;
    return 0;
  }
  //if(handle < 0) return ERRO;
  // recuperar RC de handle?
  // handle em lista de diretorios abertos
  return ERRO;
}
