OVMF = /usr/share/qemu/bios-TianoCoreEFI.bin

all: $(wildcard examples/*)

.PHONY: Makefile

%: Makefile
	@test -f $@/Makefile && make -C $@ all 2>/dev/null

test:
	qemu-system-x86_64 -bios $(OVMF) -m 64 -enable-kvm -hda fat:rw:examples
