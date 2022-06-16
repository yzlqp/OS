ARCH := aarch64-linux-gnu
CC := $(ARCH)-gcc
LD := $(ARCH)-ld
OBJDUMP := $(ARCH)-objdump
OBJCOPY := $(ARCH)-objcopy

# https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html
# -Wall [Warning Options]
# This enables all the warnings about constructions that some users consider questionable, and that are easy to avoid (or modify to prevent the warning), even in conjunction with macros. This also enables some language-specific warnings described in C++ Dialect Options 
# -g [Debugging Options]
# Produce debugging information in the operating system’s native format (stabs, COFF, XCOFF, or DWARF). GDB can work with this debugging information.
# -O0 [Optimization Options]
# Reduce compilation time and make debugging produce the expected results. This is the default.
# -O2 [Optimization Options]
# Optimize even more. GCC performs nearly all supported optimizations that do not involve a space-speed tradeoff. As compared to -O, this option increases both compilation time and the performance of the generated code.
# -fno-stack-protector [Program Instrumentation Options]
# Disable stack protection
# -static [Linker Options]
# Using static links
# -fno-builtin [C Language Options]
# Disallow the GCC compiler built-in functions
# -nostdlib [Linker Options]
# Do not link to standard library files, mainly the C runtime library
# -nostartfiles [Linker Options]
# Do not link to the startup file, mainly crtbegin.o crtend.o
# -ffreestanding [C Language Options]
# Compile separate programs that do not automatically link to C runtime library, startup files, etc  
# -mgeneral-regs-only [AArch64 Options]
# Generate code which uses only the general-purpose registers. This will prevent the compiler from using floating-point and Advanced SIMD registers but will not impose any restrictions on the assembler.
# -MMD [Preprocessor Options]
# Like -MD except mention only user header files, not system header files.
# -I dir
#
CFLAGS := -Wall -g -O0 \
          -fno-pie -fno-pic -fno-stack-protector \
          -static -fno-builtin -nostdlib -ffreestanding -nostartfiles -mcmodel=large \
          -mgeneral-regs-only \
	      -MMD -MP \
		  -Iuser/src/lib -Iuser/include -Ikernel -Ikernel/include -Ikernel/arch/aarch64/include \
		  -Ikernel/interrupt -Ikernel/drives/mmc -Ikernel/arch/aarch64/board/raspi3 -Ikernel/arch/aarch64

KERNEL_SRC_DIR := kernel
SRC_DIR := kernel
BUILD_DIR := build
USER_SRC_DIR := user
LINKER_SCRIPT := linker.ld
KERNEL_ELF := $(BUILD_DIR)/kernel8.elf
KERNEL_IMG := $(BUILD_DIR)/kernel8.img
SD_IMG := $(BUILD_DIR)/sd.img

#all: $(KERNEL_IMG)
all: $(SD_IMG)

SRCS := $(shell find $(SRC_DIR) -name *.c -or -name *.S)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)
-include $(DEPS) # 使得make根据gcc分析出的源代码依赖进行编译

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<
$(BUILD_DIR)/%.S.o: %.S
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

####################################################################################
# arch64-linux-gnu-ld
#
# Usage: aarch64-linux-gnu-ld [options] file...
# -b TARGET, --format TARGET Specify target for following input files
# -e ADDRESS, --entry ADDRESS Set start address
# -N, --omagic                Do not page align data, do not make text readonly
# -Tbss ADDRESS               Set address of .bss section
# -Tdata ADDRESS              Set address of .data section
# -Ttext ADDRESS              Set address of .text section
# -Ttext-segment ADDRESS      Set address of text segment
# -Trodata-segment ADDRESS    Set address of rodata segment
# -Tldata-segment ADDRESS     Set address of ldata segment
#
####################################################################################
# arch64-linux-gnu-objdump
#
# Usage: aarch64-linux-gnu-objdump <option(s)> <file(s)>
# -x --all-headers Display the contents of all headers
# -S Display source code and disassembly code
# -D Disassemble all segments
# -d Disassemble a segment containing machine instructions
#
####################################################################################
# aarch64-linux-gnu-objcopy
#
# Usage: aarch64-linux-gnu-objcopy [option(s)] in-file [out-file]
# -S --strip-all Remove all symbol and relocation information
# -O --output-target <bfdname> Create an output file in format <bfdname>
#
####################################################################################
# qemu-system-aarch64 [options] [disk_image]
#
# -SMP : Sets the number of CPU cores and threads
# -m : Setting The memory size
# -CPU : Sets the CPU model
# -nographic : Disable graphic output and redirect serial I/O to the console
# Linux/Multiboot boot specific:
# -kernel bzImage : use 'bzImage' as kernel image
# -append cmdline : use 'cmdline' as kernel command line
# -initrd file  　: use 'file' as initial ram disk
# -dtb    file  　: use 'file' as device tree image
# -machine : selects emulated machine ('-machine help' for list)
# -S : freeze CPU at startup (use 'c' to start execution)
# -gdb dev : accept gdb connection on 'dev'. (QEMU defaults to startingthe guest without waiting for gdb to connect; use -S tooif you want it to not start execution.)
# -s : shorthand for -gdb tcp::1234
#
####################################################################################
$(KERNEL_ELF): $(LINKER_SCRIPT) $(OBJS) $(BUILD_DIR)/$(USER_SRC_DIR)/initcode.bin
	$(LD) -T $< -o $@  $(OBJS) -b binary $(BUILD_DIR)/$(USER_SRC_DIR)/initcode.bin
	$(OBJDUMP) -S -D $@ > $(basename $@).asm
	$(OBJDUMP) -x $@ > $(basename $@).hdr

$(BUILD_DIR)/$(USER_SRC_DIR)/initcode.bin: $(USER_SRC_DIR)/initcode.S
	@echo + as $<
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $(BUILD_DIR)/$(USER_SRC_DIR)/initcode.o $<
	@echo + ld $(BUILD_DIR)/$(USER_SRC_DIR)/initcode.out
	$(LD) -N -e start -Ttext 0 -o $(BUILD_DIR)/$(USER_SRC_DIR)/initcode.out $(BUILD_DIR)/$(USER_SRC_DIR)/initcode.o
	@echo + objcopy $@
	$(OBJCOPY) -S -O binary $(BUILD_DIR)/$(USER_SRC_DIR)/initcode.out $@
	@echo + objdump $(BUILD_DIR)/$(USER_SRC_DIR)/initcode.o
	$(OBJDUMP) -S $(BUILD_DIR)/$(USER_SRC_DIR)/initcode.o > $(BUILD_DIR)/$(USER_SRC_DIR)/initcode.asm

$(KERNEL_IMG): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

#QEMU := qemu-system-aarch64 -M raspi3 -nographic -serial null -chardev stdio,id=uart1 -serial chardev:uart1 -monitor none
# Use an SD card instead
-include mksd.mk
QEMU := qemu-system-aarch64 -M raspi3 -nographic -serial null -serial mon:stdio -drive file=$(SD_IMG),if=sd,format=raw

qemu: $(KERNEL_IMG) $(SD_IMG) 
	$(QEMU) -kernel $<
qemu-gdb: $(KERNEL_IMG) $(SD_IMG)
	$(QEMU) -kernel $< -S -gdb tcp::1234
gdb: 
	gdb -x .gdbinit

.PHONY: clean test
clean:
	rm -rf $(BUILD_DIR)
test:
	echo $(USER_BIN)
