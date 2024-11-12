#include "kernel/param.h"   // Includes parameter definitions like MAXARG, which limits the number of arguments.
#include "kernel/types.h"    // Includes type definitions used in the program.
#include "kernel/stat.h"     // Includes definitions for file status and system calls.
#include "user/user.h"       // Includes user library functions for xv6.

//主函数接收命令行参数：第一个参数是追踪掩码，剩下的是要执行的参数。
int main(int argc, char *argv[])
{
  int i;                    // Variable for loop index.
  char *nargv[MAXARG];      
  // 用于保存要执行命令参数的数组。

  //检查是否提供了正确数量的参数并且追踪掩码有效，追踪掩码（argv[1]）应为‘0’到‘9’之间的单个数字。 
  if (argc < 3 || (argv[1][0] < '0' || argv[1][0] > '9')) {
    fprintf(2, "Usage: %s mask command\n", argv[0]);  
    exit(1);                                         
  }//注意，根据测试，trace 2a grep hello README这种东西是判断不出来的，也无法输出错误提醒

  // 将追踪掩码参数（argv[1]）从字符串转换为整数并传递给 `trace` 系统调用
  //如果 `trace` 失败（返回负值），则打印错误并退出。
  if (trace(atoi(argv[1])) < 0) {
    fprintf(2, "%s: trace failed\n", argv[0]);       
    exit(1);                                         
  }

  //使用从argv[2]开始的命令参数填充 `nargv` 数组。 `nargv` 将用于传递参数给exec函数。
  for (i = 2; i < argc && i < MAXARG; i++) {
    nargv[i - 2] = argv[i];                          //将参数移动到nargv的开头
  }
  nargv[argc - 2] = 0;                               //用null终止exec的参数列表

  //使用`nargv`中的参数执行 `nargv[0]` 中指定的命令，这将用新命令替换当前的进程映像。
  exec(nargv[0], nargv);

   printf("trace: exec failed\n");
  exit(1);
}
