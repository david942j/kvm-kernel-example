#ifndef PANIC_H
#define PANIC_H

/* panic occurs when assertion fails in kernel */
void __attribute__((noreturn)) panic(const char *s);

#endif
