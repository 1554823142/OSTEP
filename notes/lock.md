# new

### <stdatomic.h>

提供了对原子类型和操作的支持

- 比较并交换 Compare and Exchange

atomic_compare_exchange_strong和atomic_compare_exchange_weak函数用于比较并交换操作

如果 data->value 等于 old_value，则将 data->value 更新为 new_value，并返回 true。
如果 data->value 不等于 old_value，则不更新，返回 false。

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