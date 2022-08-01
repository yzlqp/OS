# Path of the SD card image
SD_IMG ?= $(BUILD_DIR)/sd.img
# Path of the Kernel image
KERNEL_IMG ?= $(BUILD_DIR)/kernel8.img
# Path of the boot image
BOOT_IMG := $(BUILD_DIR)/boot.img
# Path of the file system image
FS_IMG := $(BUILD_DIR)/fs.img
# The total image size is 128M in which the boot partition is 64M of our own 64M  
SECTOR_SIZE := 512
# 128 * 1024 * 1024 / 512 = 262144
SECTORS := 262144
BOOT_OFFSET := 2048
# 64 * 1024 * 1024 / 512 = 131072
BOOT_SECTORS := 131072
# 133120
FS_OFFSET := $(shell echo $$(($(BOOT_OFFSET) + $(BOOT_SECTORS))))
# 129024
FS_SECTORS := $(shell echo $$(($(SECTORS) - $(FS_OFFSET))))
BUILD_BIN_DIR := $(BUILD_DIR)/user/bin
USER_BIN := $(BUILD_BIN_DIR)/sh $(BUILD_BIN_DIR)/echo $(BUILD_BIN_DIR)/forktest $(BUILD_BIN_DIR)/hello  \
			$(BUILD_BIN_DIR)/cat $(BUILD_BIN_DIR)/ls $(BUILD_BIN_DIR)/mkdir $(BUILD_BIN_DIR)/stressfs	\
			$(BUILD_BIN_DIR)/sleep $(BUILD_BIN_DIR)/xargs $(BUILD_BIN_DIR)/find

# Delete if build fails
.DELETE_ON_ERROR: $(BOOT_IMG) $(SD_IMG)

# mkfs.fat 4.1 (2017-01-24)
# Usage: mkfs.fat [-a][-A][-c][-C][-v][-I][-l bad-block-file][-b backup-boot-sector]
#       [-m boot-msg-file][-n volume-name][-i volume-id]
#       [-s sectors-per-cluster][-S logical-sector-size][-f number-of-FATs]
#       [-h hidden-sectors][-F fat-size][-r root-dir-entries][-R reserved-sectors]
#       [-M FAT-media-byte][-D drive_number]
#       [--invariant]
#       [--help]
#       /dev/name [blocks]
# Create an empty file of the specified size (64M) with dd and format it using the FAT32 file system  
# Generating boot image
$(BOOT_IMG): $(KERNEL_IMG) $(shell find tool/boot/*)
	dd if=/dev/zero of=$@ seek=$(shell echo $$(($(BOOT_SECTORS) - 1))) bs=$(SECTOR_SIZE) count=1
	mkfs.vfat -F 32 -s 1 $@
	$(foreach x, $^, mcopy -i $@ $(x) ::$(notdir $(x));)

$(FS_IMG): $(BUILD_DIR)/user/bin/init $(USER_BIN)
	$(MAKE) -C $(USER_SRC_DIR)
	mkdir -p build/tool
	gcc -o build/tool/mkfs tool/mkfs/mkfs.c
	$(info Our filesystem files: $^)
	$(BUILD_DIR)/tool/mkfs $@ $^

$(BUILD_DIR)/user/bin/init:
	$(MAKE) -C $(USER_SRC_DIR)

$(SD_IMG): $(FS_IMG) $(BOOT_IMG)
	dd if=/dev/zero of=$@ seek=$(shell echo $$(($(SECTORS) - 1))) bs=$(SECTOR_SIZE) count=1
	@printf "\
	$(BOOT_OFFSET), $(shell echo $$(($(BOOT_SECTORS) * $(SECTOR_SIZE) / 1024)))K, c,\n\
	$(FS_OFFSET), $(shell echo $$(($(FS_SECTORS) * $(SECTOR_SIZE) / 1024)))K, L,\n\
	" | sfdisk $@
	dd if=$(BOOT_IMG) of=$@ seek=$(BOOT_OFFSET) conv=notrunc
	dd if=$(FS_IMG) of=$@ seek=$(FS_OFFSET) conv=notrunc

test2:
	echo $(USER_BIN_SRC_SD)



