CC = clang
CFLAGS = -Wall -Wextra -fsanitize=address -fsanitize=undefined
LDFLAGS = -Llib -lm -lraylib -fsanitize=address -fsanitize=undefined
SRCS = main.c
TARGET = program

default: build run

build:
	$(CC) -o $(TARGET) $(LDFLAGS) $(SRCS) $(CFLAGS)

run:
	./$(TARGET)

clean:
	rm -rf $(TARGET)


