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

init_proc: start.S init_proc.c $(LIB)/*.c
	${CC} ${CFLAGS} $(INCLUDE) -T user.ld -Wl,-Map=init_proc.map -o bin/init_proc $^
	cp bin/init_proc bin/init_proc.bin

user_shell: start.S user_shell.c $(LIB)/*.c
	${CC} ${CFLAGS} $(INCLUDE) -T user.ld -Wl,-Map=user_shell.map -o bin/user_shell $^
	cp bin/user_shell bin/user_shell.bin

objdump_time:
	${OBJDUMP} -d bin/time > time.txt
objdump_write:
	${OBJDUMP} -d bin/write > write.txt
objdump_xec:
	${OBJDUMP} -d bin/xec > xec.txt
objdump_init_proc:
	${OBJDUMP} -d bin/init_proc > init_proc.txt
objdump_user_shell:
	${OBJDUMP} -d bin/user_shell > user_shell.txt
