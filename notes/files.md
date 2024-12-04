# files

- [26.3 共享数据时产生的问题](../threads-api/p1_share_data.c)

    由于多个线程可能发生竞争状态（问题发生在临界区（访问**共享变量**的的代码段））

- [27.3 线程调用的函数返回值问题](../threads-api/bad_create_with_return.c)

    ```bash
    liubin@Y9000P:~/Downloads/ostep-code-master/threads-api$ make bad_create_with_return
    gcc -Wall -Werror -I../include     bad_create_with_return.c   -o bad_create_with_return
    bad_create_with_return.c: In function ‘mythread’:
    bad_create_with_return.c:24:12: error: function returns address of local variable [-Werror=return-local-addr]
    24 |     return (void*)&rvals;
        |            ^~~~~~~~~~~~~
    cc1: all warnings being treated as errors
    make: *** [<内置>：bad_create_with_return] 错误 1
    ```

    如果返回`栈上分配的变量`(如创建的局部变量)的指针，由于出函数就会自动的释放，所以会报错（编译器已经自动识别了）

- [27.4 线程中锁的使用](../threads-api/lock1.c)

    将锁加在临界区域， 就会保证多线程消除竞争

    ```bash
    liubin@Y9000P:~/Downloads/ostep-code-master/threads-api/build$ ./lock1
    begin main
    in the func
    begin: Thread 1 is increasinig!
    in the func
    begin: Thread 2 is increasinig!
    the sum of the counter is 20000000
    ```

