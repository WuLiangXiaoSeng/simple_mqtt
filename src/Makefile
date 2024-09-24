# 项目名称
TARGET = libmyproject

# 编译器
CC = gcc

# 编译选项
CFLAGS = -Wall -Wextra -Isrc -Isrc/log -Isrc/message -Isrc/network

# 链接选项
LDFLAGS =

# 源文件目录
SRCDIR = src

# 目标文件目录
OBJDIR = obj

# 所有源文件
SRC = $(wildcard $(SRCDIR)/*.c) \
		$(wildcard $(SRCDIR)/log/*.c) \
		$(wildcard $(SRCDIR)/message/*.c) \
		$(wildcard $(SRCDIR)/network/*.c)

# 对应的目标文件
OBJ = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC))

# 最终目标
LIB = $(TARGET).a

# 默认目标
all: $(LIB)

# 创建目标文件夹
$(OBJDIR):
	mkdir -p $(OBJDIR)

# 规则：生成静态库
$(LIB): $(OBJ)
	ar rcs $@ $^

# 规则：编译 .c 文件到 .o 文件
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# 清理规则
clean:
	rm -rf $(OBJDIR) $(LIB)

# 显示帮助信息
help:
	@echo "Usage:"
	@echo "  make        - Build the library"
	@echo "  make clean  - Clean up object files and library"