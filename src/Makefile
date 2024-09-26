CC = gcc
CFLAGS = -Wall -g -I. -Ilog -Imessage -Inetwork -fPIC
LDFLAGS = -shared  # 用于生成动态库

AR = ar
ARFLAGS = -rsv

BUILD_DIR=build
LIB_DIR=lib

# 源文件和目标文件
SRC = $(wildcard log/*.c network/*.c message/*.c)
# OBJ = $(SRC:.c=.o)
OBJ = $(foreach file,$(SRC),$(file:.c=.o))

all: libsimple_mqtt.so

$(LIB_DIR):
	mkdir -p $(LIB_DIR)

simple_mqtt.a: $(OBJ) $(LIB_DIR)
	$(AR) $(ARFLAGS) $(LIB_DIR)/$@ $(OBJ)
	rm -f $(OBJ)

libsimple_mqtt.so: $(OBJ) $(LIB_DIR)
	$(CC) $(LDFLAGS) -o $(LIB_DIR)/$@ $(OBJ)
	rm -f $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 清理生成文件
clean:
	rm -rf $(OBJ) build
