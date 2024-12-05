#include <stdio.h>
#include <stdatomic.h>
#include <stdbool.h>

typedef struct {
    atomic_int value;
} atomic_data_t;

// 模拟 StoreConditional（SC）
bool storeconditional(atomic_data_t *data, int old_value, int new_value) {
    // 使用 atomic_compare_exchange_strong 来模拟 SC 操作
    //尝试将 data->value 从 old_value 更新为 new_value，但只有在 data->value 的当前值和 old_value 相等时才会成功。
    return atomic_compare_exchange_strong(&data->value, &old_value, new_value);     //用于比较并交换操作
}

int main() {
    atomic_data_t data = { .value = ATOMIC_VAR_INIT(0) };  // 初始化数据

    // 测试 StoreConditional 操作
    int old_value = atomic_load(&data.value);  // 加载当前值

    printf("Initial value: %d\n", old_value);

    // 尝试将值更新为 1
    if (storeconditional(&data, old_value, 1)) {
        printf("Successfully updated value to 1\n");
    } else {
        printf("StoreConditional failed: value was changed by another thread\n");
    }

    // 检查更新后的值
    old_value = atomic_load(&data.value);  // 再次加载当前值
    printf("Current value after first SC attempt: %d\n", old_value);

    // 尝试将值更新为 2
    if (storeconditional(&data, old_value, 2)) {
        printf("Successfully updated value to 2\n");
    } else {
        printf("StoreConditional failed: value was changed by another thread\n");
    }

    // 最终值
    printf("Final value: %d\n", atomic_load(&data.value));

    return 0;
}
