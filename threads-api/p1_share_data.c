#include<pthread.h>
#include<stdio.h>
#include<assert.h>
#include "common_threads.h"         //其中包含更安全的自定义线程创建和join函数（包含了assert断言）

/*
    希望两个进程同时更新这个counter全局变量
*/

static volatile int counter = 0;

void* mythread(void* args){
    printf("begin: %s is increasinig!\n", (char*)args);
    int i;
    for(i = 0; i < 1e7; i++){
        counter++;
    }
    printf("%s: ok\n", (char*)args);
    return NULL;
}

int main()
{
    pthread_t p1,p2;

    printf("begin to create two threads!\n");
    Pthread_create(&p1, NULL, mythread, "A");
    Pthread_create(&p2, NULL, mythread, "B");

    
    /*
    //join 等待线程执行完毕
    线程结束后，资源并不会自动回收，必须通过 pthread_join 来回收线程资源。如果不调用 pthread_join，会导致*“僵尸线程”*存在，无法释放相关资源。
    */
    Pthread_join(p1, NULL);   
    Pthread_join(p2, NULL);
    printf("end! and the counter = %d\n",counter);
    return 0;
}

/*
运行结果1：
begin to create two threads!
begin: A is increasinig!
begin: B is increasinig!
A: ok
B: ok
end! and the counter = 10497306

运行结果2：
begin to create two threads!
begin: B is increasinig!
begin: A is increasinig!
B: ok
A: ok
end! and the counter = 10267751

都不是期望的结果！！！！！！！！
*/
