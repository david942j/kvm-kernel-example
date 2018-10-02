DIRS := hypervisor kernel

all: $(DIRS)

$(DIRS): FORCE
	$(MAKE) -C $@

debug:
	echo $(DIRS) | xargs -n 1 $(MAKE) KVM_DEBUG=1 -C

test: all
	hypervisor/hypervisor.elf kernel/kernel.bin user/orw.elf /etc/os-release

clean:
	echo $(DIRS) | xargs -n 1 $(MAKE) clean -C
FORCE:
