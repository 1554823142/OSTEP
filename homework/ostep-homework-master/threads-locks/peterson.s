# array of 2 integers (each size 4 bytes)
# load address of flag into fx register
# access flag[] with 0(%fx,%index,4)
# where %index is a register holding 0 or 1
# index reg contains 0 -> flag[0], if 1->flag[1]
.var flag   2     

# global turn variable
.var turn

# global count
.var count

.main

# put address of flag into fx
lea flag, %fx

# assume thread ID is in bx (0 or 1, scale by 4 to get proper flag address)
mov %bx, %cx   # bx: self, now copies to cx
neg %cx        # cx: - self
add $1, %cx    # cx: 1 - self   得到另一个线程的 ID

.acquire
mov $1, 0(%fx,%bx,4)    # flag[self] = 1            设置 flag[self] = 1，表示当前线程正在执行临界区
mov %cx, turn           # turn       = 1 - self     设置 turn = 1 - self，表示轮到另一个线程

.spin1
mov 0(%fx,%cx,4), %ax   # flag[1-self]              获取另一个线程的 flag（flag[1-self]），判断该线程是否在等待临界区。
test $1, %ax            
jne .fini               # if flag[1-self] != 1, skip past loop to .fini     表示另一个线程没有请求锁，可以继续执行

.spin2                  # just labeled for fun, not needed
mov turn, %ax
test %cx, %ax           # compare 'turn' and '1 - self'
je .spin1               # if turn==1-self, go back and start spin again

# fall out of spin
.fini

# do critical section now
mov count, %ax
add $1, %ax
mov %ax, count

.release
mov $0, 0(%fx,%bx,4)    # flag[self] = 0


# end case: make sure it's other's turn     将 turn 设置为 1 - self，确保轮到另一个线程
mov %cx, turn           # turn       = 1 - self
halt

