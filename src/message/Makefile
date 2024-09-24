CC = gcc
SRCDIR = $(pwd)
CFLAGS = -Wall -Wextra -I.. -I../log -I../network
OBJDIR = build
TARGET = $(OBJDIR)/message_test

SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC))
	
all: $(TARGET) $(OBJDIR)


# 创建目标文件夹
$(OBJDIR):
	mkdir -p $(OBJDIR)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)


clean:
	rm -rf $(OBJDIR)
# .DEFAULT: V = 0