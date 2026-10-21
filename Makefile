CC = clang
CFLAGS = -Wall -Wextra -I~/libs/raylib_ui/include -fsanitize=address -fsanitize=undefined
LDFLAGS = -Llib -lm -lraylib -fsanitize=address -fsanitize=undefined
SRCS = main.c ~/libs/raylib_ui/src/raylib_ui.c 
TARGET = program

default: build run

build:
	$(CC) -o $(TARGET) $(LDFLAGS) $(SRCS) $(CFLAGS)

run:
	./$(TARGET)

clean:
	rm -rf $(TARGET)


