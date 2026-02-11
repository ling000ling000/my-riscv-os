#
#CROSS_COMPILE = riscv64-unknown-elf-
#CFLAGS = -nostdlib -fno-builtin
#
## riscv64-unknown-elf-gcc 工具链可以同时编译汇编和 C 代码
#CC = ${CROSS_COMPILE}gcc
#OBJCOPY = ${CROSS_COMPILE}objcopy
#OBJDUMP = ${CROSS_COMPILE}objdump
#
#SRCS_ASM = \
#	entry.S
#
#SRCS_C = \
#	sbi.c \
#	main.c \
#
## 将源文件替换为 .o 文件
#OBJS = $(SRCS_ASM:.S=.o)
#OBJS += $(SRCS_C:.c=.o)
#
#
#os.elf: ${OBJS}
#	${CC} ${CFLAGS} -T os.ld  -o os.elf $^
#	${OBJCOPY} -O binary os.elf os.bin
#
#%.o : %.c
#	${CC} ${CFLAGS} -c -o $@ $<
#
#%.o : %.S
#	${CC} ${CFLAGS} -c -o $@ $<
#
#
#.PHONY : clean
#clean:
#	rm -rf *.o *.bin *.elf

#CROSS_COMPILE = riscv64-unknown-elf-
CROSS_COMPILE = riscv64-elf-


CC      = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump

# 编译阶段：不要 -nostdlib
#CFLAGS  = -ffreestanding -fno-builtin -O2 -g
CFLAGS  = -ffreestanding -fno-builtin -O2 -g -mcmodel=medany -Wno-int-conversion


# 链接阶段：才用 -nostdlib，避免去找 -lc/-lgloss
#LDFLAGS = -nostdlib -T os.ld
LDFLAGS = -nostdlib -T os.ld -mcmodel=medany


SRCS_ASM = \
	entry.S \
	kernel_trap.S \
	switch.S \

SRCS_C = \
	sbi.c \
	main.c \
	printf.c \
	trap.c \
	syscall.c \
	string.c \
	task.c \
	app.c \
	timer.c \
	printk.c \
	vsprintf.c \

INCLUDE:=-I./include
SRC:=./src
LIB:=./lib

OBJS = $(SRCS_ASM:.S=.o)
OBJS += $(SRCS_C:.c=.o)

os.elf: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
	$(OBJCOPY) -O binary $@ os.bin

%.o : $(SRC)/%.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $<

%.o : $(LIB)/%.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $<

%.o : $(SRC)/%.S
	${CC} ${CFLAGS} $(INCLUDE) -c -o $@ $<

.PHONY : clean
clean:
	rm -rf *.o *.bin *.elf

.PHONY : clean install

install:
	@echo "install: nothing to do"
