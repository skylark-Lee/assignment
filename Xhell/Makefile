CC = gcc
CFLAGS = -Wall -Wextra -std=gnu99 -g -D_GNU_SOURCE  # 使用gnu99并定义_GNU_SOURCE
LDFLAGS = -lm
TARGET = bin/xshell
INCLUDE = -I./include

SRCS = src/builtin.c src/execute.c src/history.c src/main.c src/parser.c \
       src/pipe.c src/redirect.c src/utils.c src/error.c src/calculator.c
OBJS = $(SRCS:src/%.c=obj/%.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p bin
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $(OBJS) $(LDFLAGS)

obj/%.o: src/%.c
	@mkdir -p obj
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -rf obj bin xshell.log

.PHONY: all clean