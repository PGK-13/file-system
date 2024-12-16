# ���ñ������ͱ���ѡ��
CC := g++
CFLAGS := -Wall -Werror -g -std=c++11 -Wno-unused-but-set-variable -Wno-unused-variable

# ��ִ���ļ�������
TARGET := fs_test

# Դ�ļ���Ŀ���ļ�
SRC := disk.cc fs.cc user.cc main.cc
OBJ := $(SRC:.cc=.o)

# ������ͷ�ļ�Ŀ¼
INCLUDES := -I.

# Ŀ��: ���ɿ�ִ���ļ�
all: $(TARGET)

# ���ɿ�ִ���ļ�
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

# ����Դ�ļ�ΪĿ���ļ�
%.o: %.cc
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# ����Ŀ���ļ������ɵĿ�ִ���ļ�
clean:
	rm -f $(OBJ) $(TARGET)

# �Զ�����������ϵ
deps: $(SRC)
	makedepend $(SRC)

# ���������ļ�
-include $(SRC:.cc=.d)
