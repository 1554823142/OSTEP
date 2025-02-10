#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char* argv[])
{
    printf("hello world (pid=%d)\n", getpid());
    int rc = fork();
    if(rc < 0){
        fprintf(stderr, "fork failure!\n");
        exit(1);
    }else if(rc == 0){          // 子进程的返回值为0
        printf("hello, I'm a child process(pid=%d)\n", (int)getpid());
    }else {
        int wc = wait(NULL);        // 父进程调用wait(), 延迟自己的执行直到子进程执行完毕, 子进程完毕wait()才返回父进程
        printf("hello, I'm parent of %d (wc=%d)(pid=%d)\n", rc, wc, (int)(getpid()));
    }
    return 0;
}