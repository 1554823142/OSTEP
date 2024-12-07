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

    这段内联汇编的解释：

    ```c
    __asm__ __volatile__ (
    " lock\n"
    " cmpxchgl %2,%1\n"
    " sete %0\n"
    : "=q" (ret), "=m" (*ptr)
    : "r" (new), "m" (*ptr), "a" (old)
    : "memory");
    ```

    - (a) `lock`
    ```asm
    " lock\n"
    ```
    `lock` 前缀指示处理器执行一个“锁”前缀操作。它使得随后的指令以原子方式执行，保证操作不会被中断。在多处理器系统中，`lock` 会确保该操作不会在执行期间被其他处理器打断，避免并发修改同一数据。

    - (b) `cmpxchgl`
    ```asm
    " cmpxchgl %2,%1\n"
    ```
    `cmpxchgl` 是 x86 指令集中的一个原子操作指令。它会比较两个值，如果它们相等，则用 `new` 的值替换 `*ptr` 的值，并返回 `*ptr` 的原始值。

    - `%2`：`new`（新值）。
    - `%1`：`*ptr`（内存地址）。
    - 如果 `*ptr` 等于 `old`，则用 `new` 替换 `*ptr`，并将 `*ptr` 的原始值存储在 `eax` 寄存器中。

    该指令执行完后，`*ptr` 会被修改为 `new`（如果原值等于 `old`），并返回原值。否则，`*ptr` 保持不变。

    - (c) `sete`
      ```asm
        " sete %0\n"
      ```
        
      `sete` 是 `set if equal` 指令，它会将 `ret` 设置为：
      - `1` 如果 `cmpxchgl` 比较时 `*ptr` 等于 `old`（即交换成功）。
      - `0` 如果 `*ptr` 不等于 `old`（即交换失败）。

    `sete` 将比较结果存储到 `ret` 中，表示 CAS 操作的成功与否。

    - 4. **输入输出约束**：
        ```c
        : "=q" (ret), "=m" (*ptr)
        : "r" (new), "m" (*ptr), "a" (old)
        : "memory");
        ```
      - 这些是 GCC 内联汇编的输入输出约束，告诉编译器如何使用寄存器和内存。

        - `=q` (ret)：`ret` 被写入，使用一个 `q`（字节寄存器）。
        - `=m` (*ptr)：表示指针 `ptr` 指向的内存将被修改。
        - `"r" (new)`：`new` 会放入一个通用寄存器中。
        - `"m" (*ptr)`：`*ptr` 也会放到内存中供操作使用。
        - `"a" (old)`：`old` 的值会被加载到 `eax` 寄存器中（这是 `cmpxchgl` 指令的要求）。     

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

