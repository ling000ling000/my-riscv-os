CROSS_COMPILE = riscv64-elf-
CFLAGS = -nostdlib -fno-builtin -mcmodel=medany -DUSER_APP

CC = ${CROSS_COMPILE}gcc
OBJCOPY = ${CROSS_COMPILE}objcopy
OBJDUMP = ${CROSS_COMPILE}objdump
INCLUDE:=-I../include

LIB = ../lib

write: start.S write.c $(LIB)/*.c
	${CC} ${CFLAGS} $(INCLUDE) -T user.ld -Wl,-Map=write.map -o bin/write $^
	cp bin/write bin/write.bin

time: start.S time.c $(LIB)/*.c
	${CC} ${CFLAGS} $(INCLUDE) -T user.ld -Wl,-Map=time.map -o bin/time $^
	cp bin/time bin/time.bin

xec: start.S xec.c $(LIB)/*.c
	${CC} ${CFLAGS} $(INCLUDE) -T user.ld -Wl,-Map=xec.map -o bin/xec $^
	cp bin/xec bin/xec.bin

objdump_time:
	${OBJDUMP} -d bin/time > time.txt
objdump_write:
	${OBJDUMP} -d bin/write > write.txt
objdump_xec:
	${OBJDUMP} -d bin/xec > xec.txt
