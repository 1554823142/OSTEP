#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    printf("hello world (pid=%d)\n", (int)getpid());

    int rc = fork();            // 创建新线程 子线程不会从头开始, 而是直接从fork()系统调用中返回
    if(rc < 0){
        fprintf(stderr, "fork failure\n");
        exit(1);
    }else if(rc == 0){          // 子进程的返回值为0
        printf("hello, I'm a child process(pid=%d)\n", (int)getpid());
    }else {
        printf("hello, I'm parent of %d (pid=%d)\n", rc, (int)(getpid()));
    }
    return 0;
}