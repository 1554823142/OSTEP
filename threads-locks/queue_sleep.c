#include<stdio.h>
#include<pthread.h>
#include"common_threads.h"
#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>
#include"queue.h"
/*
    use TEST-and-SET machanism to implete a spin lock
*/

static volatile int counter = 0;
static volatile int ready = 0;      //ready ：全局变量，控制两个线程

typedef struct
{
    int flag;           //锁是否被占用的标志
    int guard;          //自旋锁
    queue_t *q;         //存储等待锁的线程
    int futex_val;      // futex 值，用于控制 park/unpark
}lock_t;

typedef struct 
{
    lock_t* lock;
    char* str;
}args;

// park()函数：将线程挂起
void park() {
    syscall(SYS_futex, &ready, FUTEX_WAIT, 0, NULL, NULL, 0);  // 使用 futex 系统调用挂起线程
}

// unpark()函数：唤醒一个线程
void unpark(pthread_t thread) {
    syscall(SYS_futex, &ready, FUTEX_WAKE, 1, NULL, NULL, 0);  // 唤醒一个线程
}

int Test_and_Set(int* old_ptr, int new)
{
    int old = *old_ptr;
    *old_ptr = new;
    return old;
    //return __sync_lock_test_and_set(old_ptr, new);        //确保操作的原子性，并且会立即看到锁状态的变化
}

void init(lock_t* lock)
{
    lock->flag = 0;
    lock->guard = 0;
    lock->futex_val = 0; // 初始化 futex 值
    queue_init(lock->q);
}

void lock(lock_t* lock)
{
    while(Test_and_Set(&lock->guard, 1) == 1)
    ;   //spin-wait 一直利用CPU周期，直到锁可用

    if(lock->flag == 0){
        lock->flag = 1;     //设置锁被占用
        lock->guard = 0;    //释放自旋锁
    }
    else{
        queue_add(lock->q, pthread_self());             //将没获得锁的线程加入队列
        lock->guard = 0;
        park();     //休眠
    }
}

void unlock(lock_t* lock)
{
    while(Test_and_Set(&lock->guard, 1) == 1)
        ;
    if(queue_empty(lock->q))
        lock->flag = 0;
    else    
        unpark(queue_remove(lock->q));//     hold lock for next thread
    lock->guard = 0;  
}


/************************************* 
 *
*/

void* mythread(void* args_){
    args* arg = (args*)args_;
    printf(" %s is ready to use the lock!\n", arg->str);

    lock(arg->lock);
    printf("begin: %s is increasinig!\n", arg->str);

    int i;
    for(i = 0; i < 1e7; i++){
        counter++;
    }
    printf("%s: ok\n", arg->str);

    unlock(arg->lock);
    return NULL;
}






int main()
{
    lock_t lock;
    lock.q = (queue_t *)malloc(sizeof(queue_t));  // 为队列分配内存
    pthread_t p0,p1;
    init(&lock);
    args arga = {&lock, "A"};
    args argb = {&lock, "B"};
    printf("begin to create two threads!\n");

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
