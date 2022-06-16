ARCH := aarch64-linux-gnu
CC := $(ARCH)-gcc
LD := $(ARCH)-ld
OBJDUMP := $(ARCH)-objdump
OBJCOPY := $(ARCH)-objcopy

CFLAGS := -Wall -g -O0 \
          -fno-pie -fno-pic -fno-stack-protector \
          -static -fno-builtin  -ffreestanding \
          -mgeneral-regs-only \
	      -MMD -MP -Iinclude

# -e ADDRESS, --entry ADDRESS Set start address
# -N, --omagic                Do not page align data, do not make text readonly
# -Tbss ADDRESS               Set address of .bss section
# -Tdata ADDRESS              Set address of .data section
# -Ttext ADDRESS              Set address of .text section
# -Ttext-segment ADDRESS      Set address of text segment
# -Trodata-segment ADDRESS    Set address of rodata segment
# -Tldata-segment ADDRESS     Set address of ldata segment
LFLAGS := -N -e main -Ttext 0

KERNEL_SRC_DIR := ../kernel
USER_SRC_DIR := ./src
BUILD_DIR := ../build
TOOL_DIR := ../tool

USYSCALL_S := $(USER_SRC_DIR)/lib/usyscall.S
USER_LIB_SRCS := $(shell find ./src/lib -name *.c)
USER_LIB_OBJS := $(USER_LIB_SRCS:%=$(BUILD_DIR)/user/%.o)
USERDEPS := $(USER_LIB_OBJS:.o=.d)
-include $(USERDEPS) # 使得make根据gcc分析出的源代码依赖进行编译

USER_BIN_DIR := $(BUILD_DIR)/user/bin
USER_BIN_SRC := $(shell find ./src -name *.c)
USER_BIN_OBJ_TMP1 := $(filter-out ./src/init/init.c,$(USER_BIN_SRC))
USER_BIN_OBJ := $(basename $(USER_BIN_OBJ_TMP1))
USER_BIN_OBJ_FIN_T := $(notdir $(USER_BIN_OBJ))
USER_BIN_OBJ_FIN_TT := $(USER_BIN_OBJ_FIN_T:%=$(USER_BIN_DIR)/%)
USER_BIN_OBJ_FIN := $(filter-out ../build/user/bin/printf ../build/user/bin/ulib ../build/user/bin/umalloc,$(USER_BIN_OBJ_FIN_TT))

USER_BIN_OBJ_TMP2 := $(USER_BIN_SRC:%=$(USER_BIN_DIR)/%.o)
USER_BIN_OBJ_TMP3 := $(notdir $(USER_BIN_SRC))

all: $(BUILD_DIR)/user/bin/init

$(BUILD_DIR)/user/src/init/init.c.o: $(USER_SRC_DIR)/init/init.c $(USER_BIN_OBJ) $(USER_BIN_OBJ_FIN)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/user/src/lib/usyscall.S.o: $(USER_SRC_DIR)/lib/usyscall.S
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $^

$(USER_LIB_OBJS): $(USER_LIB_SRCS)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/user/src/lib/printf.c.o: $(USER_SRC_DIR)/lib/printf.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $^

$(BUILD_DIR)/user/src/lib/umalloc.c.o: $(USER_SRC_DIR)/lib/umalloc.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $^

$(BUILD_DIR)/user/src/lib/ulib.c.o: $(USER_SRC_DIR)/lib/ulib.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $^

$(BUILD_DIR)/user/bin/init: $(BUILD_DIR)/user/src/init/init.c.o $(BUILD_DIR)/user/src/lib/usyscall.S.o $(BUILD_DIR)/user/src/lib/printf.c.o $(BUILD_DIR)/user/src/lib/umalloc.c.o $(BUILD_DIR)/user/src/lib/ulib.c.o
	mkdir -p $(dir $@)
	$(LD) -N -e main -Ttext 0 -o $@ $^

$(USER_BIN_OBJ): %: %.c
	mkdir -p $(USER_BIN_DIR)
	$(CC) $(CFLAGS) -c -o ../build/user/bin/$(@F).o $<
	
$(USER_BIN_OBJ_FIN): %: %.o $(BUILD_DIR)/user/src/lib/usyscall.S.o $(BUILD_DIR)/user/src/lib/printf.c.o $(BUILD_DIR)/user/src/lib/umalloc.c.o $(BUILD_DIR)/user/src/lib/ulib.c.o
	$(LD) -N -e main -Ttext 0 -o $@ $^

.PHONY: clean test
clean:
	rm -rf $(BUILD_DIR)/user
test:
	@echo $(USER_BIN_OBJ)
	@echo $(USER_BIN_OBJ_FIN)