# Trình biên dịch
CC = gcc

# Tùy chọn biên dịch
CFLAGS = -Wall -Wextra -std=c11 -g

# File nguồn
SRC = index.c

# Tên file thực thi
TARGET = index

# Mục tiêu mặc định: biên dịch chương trình
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Xóa file biên dịch
clean:
	rm -f $(TARGET)
