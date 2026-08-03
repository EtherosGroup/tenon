CC      := gcc
LD      := ld

BUILD   := build

CFLAGS    := -m64 -mno-red-zone -ffreestanding -nostdlib -nostartfiles -nodefaultlibs \
             -Wall -Wextra -Werror -I kernel/include -mcmodel=large
LDFLAGS   := -T kernel/linker.ld -nostdlib

SRC_C   := $(shell find kernel -name '*.c')
SRC_ASM := $(shell find kernel -name '*.S')

OBJ_C   := $(patsubst %.c,$(BUILD)/%.o,$(SRC_C))
OBJ_ASM := $(patsubst %.S,$(BUILD)/%.o,$(SRC_ASM))
OBJS    := $(OBJ_ASM) $(OBJ_C)

TARGET  := $(BUILD)/kernel.elf

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(dir $(TARGET))
	$(LD) $(LDFLAGS) -o $@ $^

$(BUILD)/%.o: %.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BUILD)

run: $(TARGET)
	@echo "构建完成: $(TARGET)"
	@echo "请将 build/kernel.elf 复制到 Windows QEMU 中运行"
