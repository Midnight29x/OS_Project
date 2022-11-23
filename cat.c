#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

char buf[512];
void cat(int fd , int n_check)
{
  int n;
  int ln = 1;
  printf("\t%d\t" , ln);
  while((n = read(fd, buf, sizeof(buf))) > 0) {
    for(int i = 0; i < n ; i++)
    {
      printf("%c" , buf[i]);
      if(n_check == 1)
      {
        if(buf[i] == '\n')
        {
          ln++;
          printf("\t%d\t" , ln);
        }
      }

    }    
  }
  if(n < 0){
    fprintf(2, "cat: read error\n");
    exit(1);
  }
}

int main (int argc, char *argv[])
{
  int fd, i;
  int n_ = 0;
  int start = 0;

  if(strcmp(argv[1] , "-n") == 0)
  {
    n_ = 1;
    start = 2;
  }
  else{
    n_ = 0;
    start = 1;
  }

  if(argc <= 1){
    cat(0 , n_);
    exit(0);
  }

  for(i = start; i < argc; i++){
    if((fd = open(argv[i], 0)) < 0){
      fprintf(2, "cat: cannot open %s\n", argv[i]);
      exit(1);
    }
    cat(fd ,n_);
    close(fd);
  }
  exit(0);
}
