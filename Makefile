CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = exe
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET) $(TARGET).exe

INTERSECCAO_DEPS = interseccao_automatos.c afd.c complemento_afn.c conversor_AFN_para_AFD.c minimizar_afd.c commons.c

test-interseccao:
	$(CC) $(CFLAGS) -std=c11 -pedantic -I. tests/test_interseccao.c $(INTERSECCAO_DEPS) -o tests/test_interseccao
	./tests/test_interseccao
	$(CC) $(CFLAGS) -std=c11 -pedantic -I. tests/test_interseccao_afn.c $(INTERSECCAO_DEPS) -o tests/test_interseccao_afn
	./tests/test_interseccao_afn

.PHONY: all run clean test-interseccao
