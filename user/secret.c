#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"


int
main(int argc, char *argv[])
{
  if(argc != 2){// 检查是否传入了正确数量的参数。要求有两个参数（程序名和密钥）
    printf("Usage: secret the-secret\n");
    exit(1);
  }
  char *end = sbrk(PGSIZE*32);// 分配32页的内存空间。每页大小为PGSIZE（通常是4096字节）
  end = end + 10 * PGSIZE; // 将指针end向前移动9页（9 * PGSIZE字节），此时end指向分配空间的第9页
  strcpy(end, "my very very very secret pw is:");
  strcpy(end+32, argv[1]);// 将命令行参数argv[1]中的密钥字符串复制到end + 32位置（即在标签字符串之后的32字节位置）
  exit(0);
}

//sbrk(PGSIZE*32)将进程的数据段按照32页（为单位）增加。PGSIZE是个常数。这个调用返回一个指针，指向新分配内存起始位置。
//指针end移动9页。然后当前end指向分配内存的第9页位置。
//strcpy第一句不解释。第二句的调用，通过指令行argv[1]传递的信息secret复制到end之后32个字节位置的地方。
//所以本程序的主要行为：
//分配一个32页的内存空间，然后移动指针end，到页面的具体位置（这里是第9页）。
//它将一个字符串和用户提供的secret写入这个分配的内存。