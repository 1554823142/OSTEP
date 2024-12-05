# new

### <stdatomic.h>

提供了对原子类型和操作的支持

- 比较并交换 Compare and Exchange

atomic_compare_exchange_strong和atomic_compare_exchange_weak函数用于比较并交换操作

如果 data->value 等于 old_value，则将 data->value 更新为 new_value，并返回 true。
如果 data->value 不等于 old_value，则不更新，返回 false。