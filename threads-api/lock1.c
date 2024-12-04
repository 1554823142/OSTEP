# include<stdio.h>
#include<pthread.h>
#include"common_threads.h"

static volatile int counter = 0;

typedef struct{
    char* arg_char;
    pthread_mutex_t* lock;
}thread_args;

void* mythread(void* args)//, pthread_mutex_t* lock)
{
    printf("in the func\n");
    thread_args* t_args = (thread_args*)args;
    pthread_mutex_lock(t_args->lock);
    printf("begin: %s is increasinig!\n", t_args->arg_char);
    int i;
    for(i = 0; i < 1e7; i++){
        counter++;
    }
    pthread_mutex_unlock(t_args->lock);
    return NULL;
}

int main()
{
    printf("begin main\n");
    // the lock is needed to initialize!!!!!!!!!!!!!!
    /*
        方法1： 使用静态方法    赋值为PTHREAD_MUTEX_INITIALIZER;
        方法2： 使用动态方法    pthread_mutex_init();
    */
    pthread_mutex_t lock;
    int rc = pthread_mutex_init(&lock, NULL);   assert(rc == 0);
    // 创建并初始化线程参数
    thread_args t1_args = {"Thread 1", &lock};
    thread_args t2_args = {"Thread 2", &lock};
    pthread_t t0, t1;                   //创建两个线程
    Pthread_create(&t0, NULL, mythread, &t1_args);
    Pthread_create(&t1, NULL, mythread, &t2_args);
    
    /*
    pthread_mutex_lock(&lock);  //lock a mutex
    // the 临界区 is following 

    pthread_mutex_unlock(&lock); 
    */
      // 等待线程结束
    pthread_join(t0, NULL);
    pthread_join(t1, NULL);
    printf("the sum of the counter is %d\n", counter);
    // 销毁锁
    pthread_mutex_destroy(&lock);
    return 0;

}

