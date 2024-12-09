#include<stdio.h>
#include<pthread.h>
 #include <unistd.h>
#include"common_threads.h"

#define MAX 10
int buffer[MAX];
static int fill = 0;
static int use = 0;
static int count = 0;
static int con_loops = 10;
static int pro_loops = 30;

typedef struct 
{
    /* data */
    pthread_mutex_t* lock;
    pthread_cond_t* fill;
    pthread_cond_t* empty;
    char* str;

}my_arg;

void put(int value)
{
    buffer[fill] = value;
    fill = (fill + 1) % MAX;
    count++;
}

int get()
{
    int temp = buffer[use];
    use = (use + 1) % MAX;
    count--;
    return temp;
}

//生产者
void* producer(void* args)
{
    my_arg* arg = (my_arg*)args;
    printf("begin %s, is a producer!\n", arg->str);

    int i;
    for(i = 0; i < pro_loops; i++){
        Pthread_mutex_lock(arg->lock);
        while (count == MAX)
        {
            Pthread_cond_wait(arg->empty, arg->lock);       //等待 empty 标志
        }
        put(i);
        Pthread_cond_signal(arg->fill);                     //只会唤醒消费者
        Pthread_mutex_unlock(arg->lock);
    }
    printf("end %s\n", arg->str);
    return NULL;
}

//消费者
void* consumer(void* args)
{
    my_arg* arg = (my_arg*)args;
    printf("begin %s, is a comsumer!\n", arg->str);
    int i;
    for(i = 0; i < con_loops; i++){
        Pthread_mutex_lock(arg->lock);
        while (count == 0)
        {
            Pthread_cond_wait(arg->fill, arg->lock);
        }
        int tmp = get();
        Pthread_cond_signal(arg->empty);
        Pthread_mutex_unlock(arg->lock);
        printf("the consumer %s get the num %d\n", arg->str, tmp);
    }
    printf("end %s\n", arg->str);
    return NULL;
}

int main()
{
    printf("main begin!\n");
    pthread_mutex_t lock;
    pthread_cond_t fill, empty;
    assert(pthread_mutex_init(&lock,NULL) == 0);
    assert(pthread_cond_init(&fill,NULL) == 0);
    assert(pthread_cond_init(&empty,NULL) == 0);
    pthread_t p0,p1,p2,p3;
    my_arg args1 = {&lock, &fill, &empty, "producer1"};
    my_arg args2 = {&lock, &fill, &empty, "consumer1"};
    my_arg args3 = {&lock, &fill, &empty, "consumer2"};
    my_arg args4 = {&lock, &fill, &empty, "consumer3"};

    Pthread_create(&p0, NULL, producer, (void*)&args1);
    Pthread_create(&p1, NULL, consumer, (void*)&args2);
    Pthread_create(&p2, NULL, consumer, (void*)&args3);
    Pthread_create(&p3, NULL, consumer, (void*)&args4);
    Pthread_join(p0, NULL);
    Pthread_join(p1, NULL);
    Pthread_join(p2, NULL);
    Pthread_join(p3, NULL);
    return 0;
}