#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

char secret[8];
char output[64];

// from FreeBSD.
int
do_rand(unsigned long *ctx)
{
/*
 * Compute x = (7^5 * x) mod (2^31 - 1)
 * without overflowing 31 bits:
 *      (2^31 - 1) = 127773 * (7^5) + 2836
 * From "Random number generators: good ones are hard to find",
 * Park and Miller, Communications of the ACM, vol. 31, no. 10,
 * October 1988, p. 1195.
 */
    long hi, lo, x;

    /* Transform to [1, 0x7ffffffe] range. */
    x = (*ctx % 0x7ffffffe) + 1;
    hi = x / 127773;
    lo = x % 127773;
    x = 16807 * lo - 2836 * hi;
    if (x < 0)
        x += 0x7fffffff;
    /* Transform to [0, 0x7ffffffd] range. */
    x--;
    *ctx = x;
    return (x);
}

unsigned long rand_next = 1;

int
rand(void)
{
    return (do_rand(&rand_next));
}

// generate a random string of the indicated length.
char *
randstring(char *buf, int n)
{
  for(int i = 0; i < n-1; i++) {
    buf[i] = "./abcdef"[(rand() >> 7) % 8];// 随机选择字符放入 buf 中
  }
  if(n > 0)
    buf[n-1] = '\0';// 以空字符结尾
  return buf;
}

int
main(int argc, char *argv[])
{
  int pid;
  int fds[2];// 管道文件描述符数组

  // an insecure way of generating a random string, because xv6
  // doesn't have good source of randomness.
  rand_next = uptime();
  randstring(secret, 8);// 生成一个8字符的随机字符串存入 secret 中
  
  if((pid = fork()) < 0) {
    printf("fork failed\n");
    exit(1);   
  }
  if(pid == 0) {
    char *newargv[] = { "secret", secret, 0 };// 子进程执行 "secret" 程序，传入 secret 字符串作为参数
    exec(newargv[0], newargv); // 使用 exec 替换当前进程映像
    printf("exec %s failed\n", newargv[0]);
    exit(1);
  } else {
    wait(0);  // wait for secret to exit
    if(pipe(fds) < 0) {
      printf("pipe failed\n");
      exit(1);   
    }
    if((pid = fork()) < 0) {// 创建另一个子进程
      printf("fork failed\n");
      exit(1);   
    }
    if(pid == 0) {
      close(fds[0]);// 关闭管道的读取端
      close(2);// 关闭标准错误输出
      dup(fds[1]);// 将管道的写入端复制到文件描述符2 (标准错误输出)
      char *newargv[] = { "attack", 0 };
      exec(newargv[0], newargv);// 执行 "attack" 程序
      printf("exec %s failed\n", newargv[0]);
      exit(1);
    } else {
       close(fds[1]);// 父进程关闭管道的写入端
      if(read(fds[0], output, 64) < 0) {// 从管道中读取输出
        printf("FAIL; read failed; no secret\n");
        exit(1);
      }
      if(strcmp(secret, output) == 0) {// 比较 secret 和 output，如果相同，则输出成功信息
        printf("OK: secret is %s\n", output);
      } else {
        printf("FAIL: no/incorrect secret\n");
      }
    }
  }
}
