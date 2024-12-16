# 设置编译器和编译选项
CC := g++
CFLAGS := -Wall -Werror -g -std=c++11 -Wno-unused-but-set-variable -Wno-unused-variable

# 可执行文件的名称
TARGET := fs_test

# 源文件和目标文件
SRC := disk.cc fs.cc user.cc main.cc
OBJ := $(SRC:.cc=.o)

# 包含的头文件目录
INCLUDES := -I.

# 目标: 生成可执行文件
all: $(TARGET)

# 生成可执行文件
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

# 编译源文件为目标文件
%.o: %.cc
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 清理目标文件和生成的可执行文件
clean:
	rm -f $(OBJ) $(TARGET)

# 自动生成依赖关系
deps: $(SRC)
	makedepend $(SRC)

# 包含依赖文件
-include $(SRC:.cc=.d)
