#include<stdio.h>
#include<pthread.h>
#include"common_threads.h"
/*
    use TEST-and-SET machanism to implete a spin lock
*/

static volatile int counter = 0;

typedef struct
{
    volatile int flag;
}lock_t;

typedef struct 
{
    lock_t* lock;
    char* str;
}args;



int Test_and_Set(volatile int* old_ptr, int new)
{
    int old = *old_ptr;
    *old_ptr = new;
    return old;
    //return __sync_lock_test_and_set(old_ptr, new);        //确保操作的原子性，并且会立即看到锁状态的变化
}

void init(lock_t* lock)
{
    lock->flag = 0;
}

void lock(lock_t* lock)
{
    while(Test_and_Set(&lock->flag, 1) == 1)
    ;   //spin-wait 一直利用CPU周期，直到锁可用
}

void unlock(lock_t* lock)
{
    lock->flag = 0;
    printf("lock is unuse now!\n");
    fflush(stdout);  // 强制刷新输出缓冲区
}


/************************************* 
 *
*/

void* mythread(void* args_){
    args* arg = (args*)args_;
    printf(" %s is ready to use the lock!\n", arg->str);
    fflush(stdout);  // 强制刷新输出缓冲区
    lock(arg->lock);
    printf("begin: %s is increasinig!\n", arg->str);
    fflush(stdout);  // 强制刷新输出缓冲区
    int i;
    for(i = 0; i < 1e7; i++){
        counter++;
    }
    printf("%s: ok\n", arg->str);
    fflush(stdout);  // 强制刷新输出缓冲区
    unlock(arg->lock);
    return NULL;
}

int main()
{
    lock_t lock;
    pthread_t p0,p1;
    init(&lock);
    args arga = {&lock, "A"};
    args argb = {&lock, "B"};
    printf("begin to create two threads!\n");
    fflush(stdout);  // 强制刷新输出缓冲区
    Pthread_create(&p0, NULL, mythread, (void*)&arga);
    Pthread_create(&p1, NULL, mythread, (void*)&argb); 

    
    /*
    //join 等待线程执行完毕
    线程结束后，资源并不会自动回收，必须通过 pthread_join 来回收线程资源。如果不调用 pthread_join，会导致*“僵尸线程”*存在，无法释放相关资源。
    */
    Pthread_join(p0, NULL);   
    Pthread_join(p1, NULL);
    printf("end! and the counter = %d\n",counter);
    fflush(stdout);  // 强制刷新输出缓冲区
    return 0;

}
