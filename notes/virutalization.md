# 虚拟化(virtualizatioin)

## 概念

操作系统将物理资源(CPU, disk, memory) 转化为**更通用, 更强大, 更易于使用**的虚拟形式, 所以有时也将操作系统称为**虚拟机**

- 虚拟化CPU:

  在硬件的帮助下, OS负责提供家乡(illusion), 即系统拥有非常多虚拟CPU的假象, 将单个CPU(或者其中的一部分)转换为看似无限数量的CPU, 从而让许多程序看似同时运行

- 虚拟化内存:

  **内存就是一个字节数组**, 进行读写时必须指定地址, 虚拟化内存就是值每个进程访问自己的私有**虚拟地址空间**, 并映射到机器的物理内存

## 进程

进程就是运行中的程序, 操作系统为**正在运行的程序提供的抽象**就是进程(process)

### 机器状态

组成:

- 内存: 包含正在运行的程序和写入的数据, 所以进程可以访问的内存(即地址空间)也是该进程的一部分
- 寄存器: 指令明确的读取,更新寄存器. 一些特殊的寄存器(如PC, SP, FP(frame pointer))构成了机器状态的一部分

### 进程API

- 创建
- 销毁
- 等待
- 其他控制
- 状态

系统调用:

- [fork()系统调用](../my_code/chap5/fork_5_1.c)

- [wait()系统调用](../my_code/chap5/wait.c)

- [exec()系统调用](../my_code/chap5/exec.c) 

  让子进程父进程执行不同的程序, exec()从可执行程序中加载代码和静态数据 ,并用它覆盖自己的代码段(所以并没有创建新进程, 而是直接将当前的运行程序替换为不同的运行程序(`wc`, 也是一个程序, 使用`wc --help`查看更多), 所以子进程执行完毕后, 对exec的成功调用永远不会返回

> 上述系统与`shell`的关系:
>
> shell也是一个用户程序, 首先显示一个提示符(prompt), 后等待用户输入, shell可以在文件系统中找到可执行程序, 调用`fork()`创建新进程, 调用`exec()`的某个变体来执行这个可执行程序, 调用`wait()`等待该命令的完成. 子进程结束, shell从`wait()`返回并再次输出一个提示符, 等待用户的下一条输入

还有一点: **RTFM**(read the man)

ubuntu使用`man man`指令就可以查看, 网络上也有

### 创建过程

- 将代码和所有静态数据load到内存(更准确的是进程的地址空间)中
- 为程序的运行时栈(stack)分配内存----c中使用stack分配局部变量, 函数参数及返回地址,OS分配这些内存并提供给进程(也可能用参数初始化栈, 将参数填入main)
- 为程序的堆(heap)分配内存----c中的heap用于分配显示请求的内存

### 状态

- 运行
- 就绪(OS由于某种原因不在此时运行)
- 阻塞(I/O请求)

# 虚拟化CPU

## 受限直接执行(limited direct execution	LDE)

###  概念

让程序运行的大部分指令直接访问硬件, 只有在一些关键点(如进程发起系统调用时引起的时钟中断)由OS介入来确保在**正确的时间, 正确的地点做正确的事**

为了实现高效的虚拟化, OS应该尽量让程序自己运行, 同时通过在关键点的及时介入, 来确保对硬件的控制

#### 关键问题1: 如何执行受限的操作 ----- 使用受保护的控制权转移

硬件通过提供不同的执行模式来协助操作系统:

- 用户模式: 应用程序不能完全访问硬件资源, 例如进程不能发出I/O请求

  - ps: 管理员模式只是较高LEVEL的用户模式

- 内核模式: OS可以访问机器的全部资源, 是操作系统内核的运行方式

  要执行系统调用, 程序必须执行特殊的**陷阱(trap)**指令, 该指令同时跳入内核并升级权限级别为内核模式, 执行完后系统调用`return-from-trap`指令, 回到发起调用的用户程序中, 同时降低权限

  > 有关I/O的代码(如C中的`open(), read()`), 无法在用户态执行, 而是OS执行, 当执行代码时发现自己是用户态, 会立即发出**软中断**, 跳转到内核处理程序, 此时即为"陷入内核态", C中的系统调用部分使用汇编语言实现的, 支持了陷入OS

  执行陷阱也需要保护现场的步骤, 例如:x86处理器会将PC, 标志寄存器等推送到每个进程的**内核栈**, 从陷阱返回时会弹出这些值

#### 两个阶段

- 系统引导阶段: 内核初始化陷阱表, 方便CPU日后定位, 此时为内核特权
- 运行程序阶段: 陷阱返回指令之前内核设置分配内存等操作, 后将CPU切换到用户模式, 当运行进程希望发出系统调用时, 重新陷入OS. 整个进程执行完毕后从入口点(如`main()`函数)
- 最后, 回到内核, 释放进程中的内存, 将进程从进程列表中清除

#### 关键问题2: 进程间的切换 ------- OS如何重获CPU的控制权

- 协作方式: 等待系统调用

  OS通过等待系统调用或者非法操作(如除以0, 导致访问无法访问的内存, 就会陷入), 这样会有无限循环的风险

- 非协作方式: OS控制

  > 关键问题3 如何在没有协作的情况获得控制权

  使用**时钟中断**

- 保存和恢复上下文

  OS为当前正在执行的进程保存一些寄存器的值(到他的内核态), 并为即将执行的进程(从它的内核态)恢复寄存器的值, 这样确保了最后执行的返回陷阱指令可以继续执行另一个进程

  此协议的两种寄存器保护:

  - 时钟中断: 由硬件隐式保护
  - OS决定进程间的切换, 是显示的保存



## 调度策略

### 先进先出(FIFO)

先到先服务

### 最短任务优先(SJF)

Shortest Job First, 先运行最短的任务

### 最短完成时间优先(STCF)

Shortest Time-to-Completion First, 在当前时间剩余工作时间最短的程序可以**抢占**

### 轮转(RR)

Round-Robin, 不是直接运行一个程序到结束, 而是反复执行所有程序, 被称为时间切片, **且时间切片的长度必须是时钟中断周期的倍数**

RR在**时间响应**($$T_{响应时间}=T_{首次运行} - T_{到达时间}$$​)表现的好, 但是RR延伸每种工作的方式导致**周转周期**很差, 这也是所有**公平的策略**的特点

### 多级反馈队列(MLFQ)

Multi-level Feedback Queue, 其中有很多独立的**队列**, 每个队列有不同的**优先级**, 任何时刻一个工作只能存在于一个队列中, 总是优先执行较高优先级的工作, 每个队列中的工作优先级相同, 就采用**轮转**

#### 更加细节的调度

- 工作进入系统即为最高优先级
- 但是工作用完整个事件片后(即为长工作), 降低优先级
- 如果工作在时间片内主动释放CPU(可能是真的短, 也有可能是处理I/O(例如键盘输入)),则优先级不变, 这样的实现可以*让交互型工作快速进行*

此算法的核心是如果开始不知道工作时间的长短, 就统一假设为短, 如果为短, 则很快执行完, 否则逐渐降低优先级, 就被认为是长工作

#### 缺点

产生**饥饿**问题, 过多较短的时间工作会长时间挂起长时间工作

也会容易被攻击, 如果在时间片用完之前就调用I/O, 会独占CPU

#### 解决方法

- 周期性提升所有工作的优先级, 即经过一段时间S, 就将系统重的所有工作重新加入最高优先级队列

  这样可以保证不会发生饿死问题, 保证长时间的工作一直有进展

- 采用**更好的计时方式**, 即调度程序应该记录一个进程在某一层中**消耗的总时间**, 而不是重新计时, 进程用完自己的**配额**, 就会降低优先级(即无论配额是一次用完还是多次用完)

  这样可以避免恶意的攻击和愚弄导致的CPU垄断

- 一些命令行工具, 如`nice`, 可以稍微改变工作的优先级, (查看man手册:`man nice`)

  ```bash
  NICE(1)                          User Commands                         NICE(1)
  
  NAME
         nice - run a program with modified scheduling priority
  
  SYNOPSIS
         nice [OPTION] [COMMAND [ARG]...]
  ```



#### 小结

MLFQ***不需要**对工作的运行方式有先验知识*, 而是通过观察工作的运行来调整优先级

#### 代码实现

参照附带的[作业代码](../homework/ostep-homework-master/cpu-sched-mlfq/mlfq.py), 作者实现了完备的模拟功能



## 调度份额

调度程序的最终目标是确保每个工作获得**一定比例的CPU时间**, 而不是优化周转周期和响应时间

### 彩票调度

实现原理, 详细见[lottery.c](../cpu-sched-lottery/lottery.c)

核心代码: 

```c
// 每次循环执行.....
int counter            = 0;
int winner             = random() % gtickets; // get winner
struct node_t *current = head;

// loop until the sum of ticket values is > the winner
while (current) {
    counter = counter + current->tickets;
    if (counter > winner)
    break; // found the winner
    current = current->next;
}
// current is the winner: schedule it...
```

输出如下:(winner的第一个输出为随机中奖数winner, 第二个为当前中奖者current->tickets)

```bash
List: [25] [100] [50] 
winner: 2 25

List: [25] [100] [50] 
winner: 40 100

List: [25] [100] [50] 
winner: 43 100

List: [25] [100] [50] 
winner: 10 25

List: [25] [100] [50] 
winner: 136 50
```



### 步长调度

stride-scheduling, 确定性的公平分配算法

系统中的每个工作的**步长**(stride)和自己的票数**成反比**, 每次进程运行后, 让它的计数器(即**行程**(pass))增加它的步长, 记录它的总体进展

当需要进行调度时, 选择**目前拥有最小行程值的进程**, 并在运行之后将该进程的行程增加一个步长, 即:

```c
current = remove_min(queue);			// 选择目前拥有最小行程值的进程
schedule(current);
current->pass += current->stride;		// 在运行之后将该进程的行程增加一个步长
insert(queue, current);
```

相比于彩票调度算法, 步长调度算法可以在每个调度周期后做到票数比例的完全正确, 而彩票只是概率上实现比例

但是彩票调度算法拥有自己的优势: **不需要设置全局状态**, 只需要更新全局的总票数即可, 更加合理地**处理新加入的进程**

### 应用场景

在容易确定份额比例的场景应用较广

在虚拟数据中心, 希望分配1/4给Windows虚拟机, 剩余给Linux虚拟机



## 多处理器调度(multiprocessor scheduling)

### 问题

- 缓存一致性(cache coherence)

  不同CPU拥有自己的缓存, 其中一个修改了一个地方的内存, 但是没有写回, 另一个访问的是旧址

  基于总线的系统中,使用总线窥视(bus snooping), 每个缓存通过监听连接所有的缓存和内存的总线, 来发现内存访问

- 缓存亲合度(cache affinity)

  尽可能将进程保持在一个CPU上, 因为切换不同的CPU需要重新加载数据而变慢

### Linux多处理器调度

- 参考:<https://zhuanlan.zhihu.com/p/371855521>

- 操作系统的算法

  - O(1)

    为了解决O(n)存在的问题, 采用双队列(active(拥有时间片, 还在执行的任务)和expired(已耗尽时间片)), 耗尽时间片的任务由active转为expired状态, 重新计算优先级, 当active空了交换两个队列.

    使用bitmap结构就可以保证O(1)的复杂度, 但是存在的问题是**难以保证交互性**

  - O(n)

    只有一个全局的队列queue, 每次调度时都会遍历队列来寻找下一个要调度的任务, 但是不可拓展, 直到Linux2.4都在使用, 任务变多时会出现调度时间过长的问题, 并且**随着CPU数目的增大, 资源浪费越来越严重**(因为如果其中的一个CPU耗尽了自己的时间片, 就只能等待其他的CPU运行任务的结束, 才能重新调度)

#### 完全公平调度(Completely Fair Scheduler, CFS)

- 参考:<https://zhuanlan.zhihu.com/p/372441187>
- Linux内核api: <https://www.kernel.org/doc/html/latest/core-api/index.html>

为O(1)调度器, 基于**加权公平排队思想**的调度, 并采用**红黑树**作为调度任务队列保证了精确性

CFS使用红黑树存储调度任务队列, 每个节点代表要调度的任务, 节点的key为**虚拟时间**(vruntime), key越小, 节点越靠左, 而CFS每次挑选最左边的节点作为下一个要运行的任务, 由于该节点被特殊指针保存而被**缓存**, 所以搜索时间为O(1)

- `vruntime`

  - 含义:

    Linux 内核调度器中用于表示进程运行时间的一种机制, 每个进程都会有一个 `vruntime` 值，它表示进程自启动以来“消耗”了多少调度时间, `vruntime` 越小的进程，优先级越高，越有可能被调度执行

  - 计算:

    ```javascript
    vruntime += 实际运行时间(time process run) * 1024 / 进程权重
    ```

    - 进程权重:

      根据进程的 `nice` 值来计算, `nice` 值是操作系统用于决定**进程优先级**的一个机制，范围通常是 `-20` 到 `19`。`nice` 值越小，优先级越高，权重越低，进程的 `vruntime` 增加得也更慢，意味着该进程更有可能被调度

      

对于新任务来说, vruntime=0, 此特性也为CFS**抢占特性**:

休眠进程在唤醒时会获得vruntime步长, 在醒来的时候有大概率能力抢占CPU, 即保证交互式进程的响应速度(因为交互式进程会在等待用户输入时频繁休眠)



#### 脑残调度器(Brain Fuck Scheduler, BFS)

适用于低端的, CPU数量较少的设备上表现良好



# 虚拟化内存

## 地址空间

address space, 运行程序看到的系统中的内存, 其组成部分粗略分为:

- 代码
- 堆
- 栈

### 虚拟内存

#### 目标

- 透明:

  保证程序对实现虚拟内存方式的**不可见**, 让程序认为自己有私有的物理内存

- 效率:

  时间和空间的高效, 有时依赖硬件(如TLB的硬件功能)

  TLB(translation lookaside buffer): 一种高速缓存, 改善虚拟地址到物理地址的转换速度

- 保护:

  进程之间不会相互影响, OS本身也不会受到进程的影响, 保护让我们能够在进程间提供**隔离**

#### 底层操作系统支持

`malloc()` 分配小内存时通常使用 `brk/sbrk` 调整堆顶，分配大块内存（如超过`MMAP_THRESHOLD`，默认 128KB）时可能底层调用 `mmap()` 创建匿名映射。

使用`mmap()`调用从OS中获得内存, 可以在程序中创建一个**匿名**内存区域, 此区域与**交换空间**相关联

- `mmap()`:

   memory map, 用于将文件或设备映射到进程的虚拟地址空间, 当使用mmap映射文件到进程后，就可以直接操作这段虚拟地址进行文件的读写等操作，*不必再调用read，write等系统调用*

- 匿名内存区域:

  不会依赖任何文件, 声明周期与进程绑定, 进程退出有OS释放

- 交换空间:

  OS用于拓展物理内存的机制, 当系统的物理内存不足时，操作系统会将部分不活跃的内存页（Memory Pages）从 RAM 移动到磁盘上的交换空间，从而腾出物理内存供其他进程使用



其他: c++内存泄漏检测工具-----**Valgrind**

例如一个缺陷的程序:

```c
#include<stdio.h>
#include<stdlib.h>

int main()
{
    int* array = malloc(sizeof(int) * 10);

}
```

使用gdb不会发现错误, 而使用Valgrind:

```bash
liubin@Y9000P:~/Desktop$ valgrind --leak-check=yes ./test
==14375== Memcheck, a memory error detector
==14375== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
==14375== Using Valgrind-3.18.1 and LibVEX; rerun with -h for copyright info
==14375== Command: ./test
==14375== 
==14375== 
==14375== HEAP SUMMARY:
==14375==     in use at exit: 40 bytes in 1 blocks
==14375==   total heap usage: 1 allocs, 0 frees, 40 bytes allocated
==14375== 
==14375== 40 bytes in 1 blocks are definitely lost in loss record 1 of 1
==14375==    at 0x4848899: malloc (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==14375==    by 0x10915E: main (in /home/liubin/Desktop/test)
==14375== 
==14375== LEAK SUMMARY:
==14375==    definitely lost: 40 bytes in 1 blocks
==14375==    indirectly lost: 0 bytes in 0 blocks
==14375==      possibly lost: 0 bytes in 0 blocks
==14375==    still reachable: 0 bytes in 0 blocks
==14375==         suppressed: 0 bytes in 0 blocks
==14375== 
==14375== For lists of detected and suppressed errors, rerun with: -s
==14375== ERROR SUMMARY: 1 errors from 1 contexts (suppressed: 0 from 0)
```

- 其中可以显示alloc了几块, free了几块, 以及分配的块的大小
- 开始的数字`==14375==`为进程ID
- `at 0x4848899: malloc (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)`: 表明内存泄漏发生在malloc()函数中, `0x4848899` 是 `malloc()` 函数在内存中的地址, 后面的.so是Valgrind 用于替换标准内存管理函数的库
- `by 0x10915E: main (in /home/liubin/Desktop/test)`: 表明内存泄漏发生在main中, `0x10915E` 是 `main()` 函数中调用 `malloc()` 的地址

再如下面的例子:

```c
#include<stdio.h>
#include<stdlib.h>

int main()
{
    int* array = malloc(sizeof(double) * 100);
    char* string = malloc(sizeof(char) * 10); 
    int* array2 = malloc(sizeof(int) * 10);
    free(array);

}
```

输出为:

```bash
==14909== HEAP SUMMARY:
==14909==     in use at exit: 50 bytes in 2 blocks
==14909==   total heap usage: 3 allocs, 1 frees, 850 bytes allocated
==14909== 
==14909== 10 bytes in 1 blocks are definitely lost in loss record 1 of 2
==14909==    at 0x4848899: malloc (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==14909==    by 0x10918C: main (in /home/liubin/Desktop/test)
==14909== 
==14909== 40 bytes in 1 blocks are definitely lost in loss record 2 of 2
==14909==    at 0x4848899: malloc (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==14909==    by 0x10919A: main (in /home/liubin/Desktop/test)
==14909== 
==14909== LEAK SUMMARY:
==14909==    definitely lost: 50 bytes in 2 blocks
==14909==    indirectly lost: 0 bytes in 0 blocks
==14909==      possibly lost: 0 bytes in 0 blocks
==14909==    still reachable: 0 bytes in 0 blocks
==14909==         suppressed: 0 bytes in 0 blocks
==14909== 
==14909== For lists of detected and suppressed errors, rerun with: -s
==14909== ERROR SUMMARY: 2 errors from 2 contexts (suppressed: 0 from 0)
```

#### 注意

程序中打印的地址均为虚拟地址, 只有OS(和硬件)才知道物理地址

## 地址转换

硬件将指令中的虚拟地址转化为数据实际存储的物理地址, 同时OS也应该设置好硬件, 同时**管理内存**, 记录被占用和空闲的内存位置.

### 动态(基于硬件)重定位

即机制加界限(base and bound)机制, 每个CPU需要base和bound寄存器, 这可以使我们将地址空间放在物理地址的任何位置, 同时确保进程只能访问自己的地址空间, 即:

```javascript
physical address = virtual address + base
```

其中地址寄存器用来转换为物理地址, 界限寄存器用来保证地址在进程地址的空间范围

其中[作业](../homework/ostep-homework-master/vm-mechanism/relocation.py)中的运行结果如下:

```bash
ARG seed 0
ARG address space size 1k
ARG phys mem size 16k

Base-and-Bounds register information:

  Base   : 0x00003082 (decimal 12418)
  Limit  : 472

Virtual Address Trace
  VA  0: 0x000001ae (decimal:  430) --> VALID: 0x00003230 (decimal: 12848)
  VA  1: 0x00000109 (decimal:  265) --> VALID: 0x0000318b (decimal: 12683)
  VA  2: 0x0000020b (decimal:  523) --> SEGMENTATION VIOLATION
  VA  3: 0x0000019e (decimal:  414) --> VALID: 0x00003220 (decimal: 12832)
  VA  4: 0x00000322 (decimal:  802) --> SEGMENTATION VIOLATION
```



CPU负责地址转换的部分称为**内存管理单元(MMU)**

#### 软件重定位

也称为静态重定位, 名为加载程序的软件接手将要运行的可执行程序, 但是问题是不提供访问保护, 这时就需要硬件支持

### 分段

给地址空间内的每个逻辑段(segment, 典型的地址空间中三个不同的逻辑段: 代码, 栈和堆)一对基址和界限寄存器对, 分段的机制可以OS能够将不同的短放到不同的物理内存区域, 从而避免了虚拟地址空间中未使用部分占用物理内存

#### 段错误(segment fault)

**试图访问非法的地址**, 如地址越界, 就会陷入OS, 终止出错进程

eg:

```c
#include <stdio.h>

int main() {
    int *ptr = NULL; // 指针指向NULL
    *ptr = 10;       // 尝试对NULL指针解引用
    return 0;
}
```

输出表明:

```bash
段错误 (核心已转储)
```

其他的情况还有:

1. **访问空指针**：尝试对空指针解引用。
2. **数组越界**：访问数组的越界元素。
3. **野指针**：访问已释放的内存或未初始化的指针。
4. **递归调用过深**：栈空间耗尽。
5. **非法内存访问**：尝试访问非法的内存区域。

### 栈的反向增长

与堆和代码段不同, 栈通常是**从高地址向低地址增长**, 为了提高内存管理的效率

### 代码实现

[示例代码](../homework/ostep-homework-master/vm-segmentation/segmentation.py)

核心实现代码:

```c
paddr = 0
        if (vaddr >= (asize / 2)):
            # seg 1
            #  [base1+len1]  [negative offset]
            paddr = nbase1 + (vaddr - asize)
            if paddr < base1:
                print('  VA %2d: 0x%08x (decimal: %4d) --> SEGMENTATION VIOLATION (SEG1)' % (i, vaddr, vaddr))
            else:
                print('  VA %2d: 0x%08x (decimal: %4d) --> VALID in SEG1: 0x%08x (decimal: %4d)' % (i, vaddr, vaddr, paddr, paddr))
        else:
            # seg 0
            if (vaddr >= len0):
                print('  VA %2d: 0x%08x (decimal: %4d) --> SEGMENTATION VIOLATION (SEG0)' % (i, vaddr, vaddr))
            else:
                paddr = vaddr + base0
                print('  VA %2d: 0x%08x (decimal: %4d) --> VALID in SEG0: 0x%08x (decimal: %4d)' % (i, vaddr, vaddr, paddr, paddr))
```



输出如下:

```bash
ARG seed 0
ARG address space size 1k
ARG phys mem size 16k

Segment register information:

  Segment 0 base  (grows positive) : 0x00001aea (decimal 6890)
  Segment 0 limit                  : 472

  Segment 1 base  (grows negative) : 0x00001254 (decimal 4692)
  Segment 1 limit                  : 450

Virtual Address Trace
  VA  0: 0x0000020b (decimal:  523) --> SEGMENTATION VIOLATION (SEG1)
  VA  1: 0x0000019e (decimal:  414) --> VALID in SEG0: 0x00001c88 (decimal: 7304)
  VA  2: 0x00000322 (decimal:  802) --> VALID in SEG1: 0x00001176 (decimal: 4470)
  VA  3: 0x00000136 (decimal:  310) --> VALID in SEG0: 0x00001c20 (decimal: 7200)
  VA  4: 0x000001e8 (decimal:  488) --> SEGMENTATION VIOLATION (SEG0)
```

- 由于段0和段1是靠最高位区分, 如VA 0, 最高位为1, 则为SEG14

- 关于段1的有效地址范围:

  - **起始地址**：`0x00001254 - 450 = 0x00001004`（十进制 4084）
  - **结束地址**：`0x00001254`（十进制 4692）

- 关于VA 1的不合法:

  ```
  物理地址 = 基址 - (段1限制 - 虚拟地址)
  		= 0x00001254 - (450 - 523) = 0x000012A3
  ```

  

- 关于VA 2的合法问题: 

  ```javascript
  物理地址 = 基址 - (段1限制 - 虚拟地址)
  	    = 0x00001254 - (450 - 802) = 0x00001176
  ```

  
