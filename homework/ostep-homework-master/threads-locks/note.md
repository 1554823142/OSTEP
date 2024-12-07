## flag.s

- `flag.s`:<br>
    ```txt
    .var flag
    .var count

    .main
    .top

    .acquire
    mov  flag, %ax      # get flag(to ax)
    test $0, %ax        # if we get 0 back: lock is free!
    jne  .acquire       # if not, try again
    mov  $1, flag       # store 1 into flag

    # critical section(临界区)
    mov  count, %ax     # get the value at the address(count -> ax)
    add  $1, %ax        # increment it                (ax++)
    mov  %ax, count     # store it back               (ax -> count)

    # release lock
    mov  $0, flag       # clear the flag now

    # see if we're still looping
    sub  $1, %bx
    test $0, %bx
    jgt .top

    halt
    ```
    这段程序实现了一个加锁的累加器，`.acquire`部分通过不断检验`ax`是否为0来确定是否获得锁，如果获得则设置`flag(是否获得锁的标志)`为1,否则就一直等待获得锁

    注意count是存储在`全局或静态存储区`,它将存储在数据段（通常是全局变量区）中，程序的整个运行期间都可以访问

## test-and-set.s

- `xchg指令`</br>
xchg 指令：是 "交换"（exchange）的缩写，它将操作数的值交换位置。对于一个寄存器和内存位置，它的效果是：
    - 将寄存器中的值交换到内存位置。
    - 将内存位置的值交换到寄存器中。</br>
xchg 是一个`原子操作（atomic operation）`，它保证了在交换值的过程中，其他线程不能干扰。这使得它成为实现锁机制的理想选择。通过使用 xchg，可以避免使用多个指令进行操作时产生的竞争条件。

- `test-and-set.s`:</br>
  ```txt
    .var mutex
    .var count

    .main
    .top	

    .acquire
    mov  $1, %ax        
    xchg %ax, mutex     # atomic swap of 1 and mutex
    test $0, %ax        # if we get 0 back: lock is free!
    jne  .acquire       # if not, try again

    # critical section
    mov  count, %ax     # get the value at the address
    add  $1, %ax        # increment it
    mov  %ax, count     # store it back

    # release lock
    mov  $0, mutex

    # see if we're still looping
    sub  $1, %bx
    test $0, %bx
    jgt .top	

    halt
  ```
  `acquire`段：如果交换前 mutex 的值是 0（锁空闲），则 %ax 会变成 0，表示获取锁成功
; 如果交换前 mutex 的值是 1（锁已被占用），则 %ax 会变成 1，表示锁仍然被占用

## ticket.s

- `fetchadd`:</br>
这条指令是 "fetch and add"（取值并加）的缩写。它将一个值加到指定的内存位置，并返回该内存位置的`原始值`。