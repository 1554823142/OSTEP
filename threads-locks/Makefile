CC     := gcc
CFLAGS := -Wall -Werror -I../include 

SRCS   := compare-and-swap.c \
		  test-and-set.c	\
		  LL_SC.c	\
		  fetch-and-add.c \
		  test-and-set2.c	\
		  queue_sleep.c \
		  bounded_buffer.c

# 设置输出目录
OUT_DIR := build

# 设置源文件对应的目标文件
OBJS   := $(SRCS:.c=.o)
OBJS   := $(OBJS:%=$(OUT_DIR)/%)

# 设置程序文件名
PROGS  := $(SRCS:.c=)
PROGS  := $(PROGS:%=$(OUT_DIR)/%)

# 默认目标
.PHONY: all
all: $(PROGS)

# 编译生成可执行文件
$(OUT_DIR)/%: $(OUT_DIR)/%.o Makefile
	$(CC) $< -o $@ -pthread

# 清理
clean:
	rm -f $(OUT_DIR)/*

# 编译生成目标文件
$(OUT_DIR)/%.o: %.c Makefile
	mkdir -p $(OUT_DIR)  # 确保目标文件夹存在
	$(CC) $(CFLAGS) -c $< -o $@
