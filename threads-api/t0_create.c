#include<pthread.h>
#include<stdio.h>
#include<assert.h>

void* mythread(void* myargs)
{
    printf("%s\n", (char*)myargs);
    return NULL;
}

int main()
{
    
    /* Thread identifiers.  The structure of the attribute type is not
    exposed on purpose.  
    
    typedef unsigned long int pthread_t;*/
    pthread_t p1,p2;
    int rc;         //rc 用来获得线程函数的返回值， 由此来进行状态的判断

    /* Create a new thread, starting with execution of START-ROUTINE
   getting passed ARG.  Creation attributed come from ATTR.  The new
   handle is stored in *NEWTHREAD.  */
// extern int pthread_create (pthread_t *__restrict __newthread,
// 			   const pthread_attr_t *__restrict __attr,
// 			   void *(*__start_routine) (void *),
// 			   void *__restrict __arg) __THROWNL __nonnull ((1, 3));
//void *(*start_routine)(void *)：线程启动时执行的函数，必须是一个返回 void* 并接受一个 void* 类型参数的函数
    printf("begin to create two threads!\n");
    rc = pthread_create(&p1, NULL, mythread, "A"); assert(rc == 0);     //如果创建线程成功，则返回0；否则返回错误码
    rc = pthread_create(&p2, NULL, mythread, "B"); assert(rc == 0);

    
    /*
    //join 等待线程执行完毕
    线程结束后，资源并不会自动回收，必须通过 pthread_join 来回收线程资源。如果不调用 pthread_join，会导致*“僵尸线程”*存在，无法释放相关资源。
    */
    rc = pthread_join(p1, NULL);    assert(rc == 0);        
    rc = pthread_join(p2, NULL);    assert(rc == 0); 
    printf("end!\n");
    return 0;
}