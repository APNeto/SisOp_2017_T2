#include <stdio.h>
#include <stdlib.h>


int main(){
char name = "/";
char *pathname;
pathname = name;
int result = opendir2(pathname);
printf("%d\n",result);
}
