CROSS_COMPILE = riscv64-elf-
CFLAGS = -nostdlib -fno-builtin -mcmodel=medany

CC = ${CROSS_COMPILE}gcc
OBJCOPY = ${CROSS_COMPILE}objcopy
OBJDUMP = ${CROSS_COMPILE}objdump
INCLUDE:=-I../include

LIB = ../lib

all: write time

bin:
	mkdir -p bin

write: write.c $(LIB)/*.c | bin
	${CC} ${CFLAGS} $(INCLUDE) -T user.ld -o bin/write.bin $^

time: time.c | bin
	${CC} ${CFLAGS} $(INCLUDE) -T user.ld -o bin/time.bin $^

.PHONY: all
