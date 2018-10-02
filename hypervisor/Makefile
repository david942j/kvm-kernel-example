TARGET := hypervisor.elf
SRCS := hypervisor.c hypercall.c
DEPS := definition.h debug.h
CFLAGS := -Wall

ifdef KVM_DEBUG
CFLAGS += -DDEBUG
endif

all: $(TARGET)

$(TARGET): $(SRCS) $(DEPS)
	$(CC) $^ $(CFLAGS) -o $@

debug:
	$(MAKE) KVM_DEBUG=1

clean:
	$(RM) $(TARGET)
