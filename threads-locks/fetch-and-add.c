#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include"common_threads.h"
/*
    use FETCH-and-ADD machanism to implete a spin lock
*/

static volatile int counter = 0;

typedef struct
{
    int ticket;
    int turn;
}lock_t;

typedef struct 
{
    lock_t* lock;
    char* str;
}args;



int FetchAndAdd(int* ptr)
{
    int old = *ptr;
    *ptr = old + 1;
    return old;
    //return __sync_lock_test_and_set(old_ptr, new);        //确保操作的原子性，并且会立即看到锁状态的变化
}

void init(lock_t* lock)
{
    lock->ticket = 0;
    lock->turn = 0;
}

void lock(lock_t* lock)
{
    int my_turn = FetchAndAdd(&lock->ticket);
    while(my_turn != lock->turn)            //如果轮次不匹配，表示锁被其他线程占用，继续自旋
    ;   //spin-wait 一直利用CPU周期，直到锁可用
}

void unlock(lock_t* lock)
{
    FetchAndAdd(&lock->turn);
}


/************************************* 
 *
*/

void* mythread(void* args_){
    args* arg = (args*)args_;
    printf(" %s is ready to use the lock!\n", arg->str);
    fflush(stdout);  // 强制刷新输出缓冲区
    printf("before the lock: %s thread's ticket is %d, and the turn is %d\n", arg->str, arg->lock->ticket, arg->lock->turn);
    lock(arg->lock);
    printf("begin: %s is increasinig!\n", arg->str);
    printf("in the lock: %s ,now and the turn is %d\n", arg->str, arg->lock->turn);
    fflush(stdout);  // 强制刷新输出缓冲区
    int i;
    for(i = 0; i < 1e7; i++){
        counter++;
    }
    sleep(5);
    printf("%s: ok\n", arg->str);
    fflush(stdout);  // 强制刷新输出缓冲区
    unlock(arg->lock);
    return NULL;
}

int main()
{
    lock_t lock;
    pthread_t p0,p1,p2,p3;
    init(&lock);
    args arga = {&lock, "A"};
    args argb = {&lock, "B"};
    args argc = {&lock, "C"};
    args argd = {&lock, "D"};
    printf("begin to create two threads!\n");
    fflush(stdout);  // 强制刷新输出缓冲区
    Pthread_create(&p0, NULL, mythread, (void*)&arga);
    Pthread_create(&p1, NULL, mythread, (void*)&argb); 
    Pthread_create(&p2, NULL, mythread, (void*)&argc);
    Pthread_create(&p3, NULL, mythread, (void*)&argd);

    
    /*
    //join 等待线程执行完毕
    线程结束后，资源并不会自动回收，必须通过 pthread_join 来回收线程资源。如果不调用 pthread_join，会导致*“僵尸线程”*存在，无法释放相关资源。
    */
    Pthread_join(p0, NULL);   
    Pthread_join(p1, NULL);
    Pthread_join(p2, NULL);   
    Pthread_join(p3, NULL);
    printf("end! and the counter = %d\n",counter);
    fflush(stdout);  // 强制刷新输出缓冲区
    return 0;

}
