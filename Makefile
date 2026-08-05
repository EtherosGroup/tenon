CC = gcc
LD = ld

CFLAGS = -m64 -mno-red-zone -ffreestanding -nostdlib -nostartfiles \
         -nodefaultlibs -Wall -Wextra -Werror -O0 \
         -I kernel/include \
         -mcmodel=large

LDFLAGS = -T kernel/linker.ld -nostdlib

SRC_C = $(shell find kernel -name '*.c')
SRC_S = $(shell find kernel -name '*.S')
OBJ_C = $(patsubst kernel/%.c, build/kernel/%.o, $(SRC_C))
OBJ_S = $(patsubst kernel/%.S, build/kernel/%.o, $(SRC_S))
OBJ   = $(OBJ_C) $(OBJ_S)

.PHONY: all run clean iso

all: build/kernel.elf

build/kernel/%.o: kernel/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

build/kernel/%.o: kernel/%.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

build/kernel.elf: $(OBJ)
	$(LD) $(LDFLAGS) $(OBJ) -o $@

iso: all
	@mkdir -p iso/boot
	cp build/kernel.elf iso/boot/
	grub-mkrescue -o tenon.iso iso/ 2>/dev/null
	@echo "ISO generated: tenon.iso"

run:
	@echo "kernel built: build/kernel.elf"
	@echo "Copy to Windows QEMU to run."

clean:
	rm -rf build/ tenon.iso
