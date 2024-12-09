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

- [27.5 线程中的条件变量使用](../threads-api/cv1.c)

    条件变量是一个`显式队列`, 当某些条件不满足时， 线程可以把自己加入队列，等待该条件；另外的线程可以通过更改上述的状态，发出信号(`signal`)来唤醒其他的线程

    设置为wait的线程休眠，直到其他的线程唤醒它， 且该wait线程在休眠时释放锁;当线程被唤醒时，重新获得锁

    - `使用while(done == 0)的原因`
        - `为什么不直接省略done`:
            父线程只能等待子线程唤醒自己， 不能判断子线程是否完成，如果没有done，则父线程就会发生没必要的等待
        - `为什么不用if`:防止子线程修改完done，发出信号时，如果此时父线程被中断，就会忽略这条信号，导致长眠不醒

- [28.5 test-and-set lock](../threads-locks/test-and-set.c)

    实现一个简单的自旋锁

    核心代码`test-and-set`:

    ```cpp
        int Test_and_Set(int* old_ptr, int new)
        {
            int old = *old_ptr;
            *old_ptr = new;
            return old;
        }
    ```

    - `问题`：</br>
        但是试了很多次只有一次完全的执行完毕，并且得到了正确的结果，其他很多次都是进行完一个线程就被阻塞了,gpt说是`锁释放后线程 B 没有及时看到锁的状态变化，可能与内存可见性问题相关`

- [28.5 compare-and-swap](../threads-locks/compare-and-swap.c)

- [链接的加载和条件存储指令实现锁](../threads-locks/LL_SC.c)

    - `store-conditonal指令`</br>
        只有上一次的加载的地址在期间没有更新时才会成功
- [28.11 获取并增加](../threads-locks/fetch-and-add.c)
    当线程的`myturn == turn`时，就会轮到该线程进入临界区
    - 数据结构：
    ```c
        typedef struct
    {
        int ticket; //表示正在排队的票据编号。每个线程会获取一个票据编号，线程按票据编号的顺序获取锁。
        int turn;   //表示当前轮次，指示哪个线程能够获取锁。
    }lock_t;
    ```
    - 实现原理

        - 第一个线程调用 FetchAndAdd(&lock->ticket)，得到票号 0。然后它进入自旋等待，直到 turn == 0，才会进入临界区执行任务。
        - 第二个线程调用 FetchAndAdd(&lock->ticket)，得到票号 1。它在 turn == 1 时才会获得锁。
        - 其他线程依此类推，都会等待 turn 与它们的票号一致时，才能获取锁。
        - `注意`： 线程之间是共享堆空间的！！！！！！！！！！！！！
    - 优点</br>
    本方法可以保证所有的线程均会得到锁，只要一个线程获得了ticket,就最终会被调度
- [28.13 让出来！](../threads-locks/test-and-set2.c)
    将自旋（即while中无内容）用yield（）代替，这时就不会出现原来版本的卡死问题

- [28.14 使用队列： 休眠替代自旋](../threads-locks/queue_sleep.c)

- [30.2 生产者/消费者（有界缓冲区）问题](../threads-locks/bounded_buffer.c)
    也叫做生产者/消费者问题：生产者把数据放在缓冲区，而消费者从缓冲区中取走数据</br>
    eg:使用管道连接不同程序的输入输出时，也会使用buffer(eg: grep foo file.txt | wc -l)，上一个指令的输出作为下一个指令的输入，其中的`共享资源就是有界缓冲区`.

    - `Mesa语义`：  `保证在使用条件变量时使用while循环`

        如果使用if来判断缓冲区是空是满，则会发生如果多个线程被唤醒，在一个线程运行之前，另一个消费者已经改变了缓冲区，导致该线程无法获取数据；也就是说，发出的信号只是保证了状态发生了变化，而`无法保证`在接受到信号的一直是期望的状态

        使用`while`可以在线程被唤醒时再次检查共享变量，确保条件确实满足，只有在条件满足时才会继续执行
    - `使用两个条件变量`

        消费者和生产者各等待一个条件变量，避免了消费者不会唤醒消费者（因为可能会唤醒生产者）