#include<stdio.h>
#include<pthread.h>
 #include <unistd.h>
#include"common_threads.h"

static volatile int ready = 0;      //ready ：全局变量，控制两个线程

typedef struct 
{
    /* data */
    pthread_mutex_t* lock;
    pthread_cond_t* cond;
    char* str;

}my_arg;

void* mythread1(void* args)
{
    //等待另一个线程将ready置为1
    my_arg* arg = (my_arg*)args;
    printf("begin %s\n", arg->str);
    Pthread_mutex_lock(arg->lock);
    while(ready == 0){
        Pthread_cond_wait(arg->cond,arg->lock);     //相比sigbal传入一个lock参数是为了在等待的时候释放锁
    }
    Pthread_mutex_unlock(arg->lock);
    printf("end %s\n", arg->str);
    return NULL;
}

void* mythread2(void* args)
{
    my_arg* arg = (my_arg*)args;
    printf("begin %s\n", arg->str);
    Pthread_mutex_lock(arg->lock);
    ready = 1;
    printf("begin sleep for 4 sec!\n");
    sleep(4);
    Pthread_cond_signal(arg->cond);
    Pthread_mutex_unlock(arg->lock);
    printf("end %s\n", arg->str);
    return NULL;
}

int main()
{
    printf("main begin!\n");
    pthread_mutex_t lock;
    pthread_cond_t cond;
    assert(pthread_mutex_init(&lock,NULL) == 0);
    assert(pthread_cond_init(&cond,NULL) == 0);
    pthread_t p0,p1;
    my_arg args1 = {&lock, &cond, "mythread1"};
    my_arg args2 = {&lock, &cond, "mythread2"};
    Pthread_create(&p0, NULL, mythread1, (void*)&args1);
    Pthread_create(&p1, NULL, mythread2, (void*)&args2);
    Pthread_join(p0, NULL);
    Pthread_join(p1, NULL);
    return 0;
}