CSRCS	:= $(wildcard *.c)
SSRCS	:= $(wildcard *.s)
OBJS	:= $(SSRCS:.s=.o) $(CSRCS:.c=.o)
DEPS	:= $(CSRCS:.c=.d)

CFLAGS := -nostdlib -Os -Wall -Werror -fPIE -pie -masm=intel -I..

all: $(TARGET)

-include $(DEPS)

$(TARGET): $(OBJS)
	$(AR) rcs $@ $^

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -MMD -MP $<

%.o: %.s
	$(AS) $^ -o $@

.PHONY: clean
clean:
	$(RM) $(DEPS) $(OBJS) $(TARGET)
