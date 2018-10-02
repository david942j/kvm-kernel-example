#!/usr/bin/env python2

from pwn import *
import os

context.arch = 'amd64'

def generate(filename, data):
    elf = make_elf(data)
    path = os.path.join(os.path.dirname(os.path.realpath(__file__)), filename)
    open(path, 'wb').write(elf)
    os.chmod(path, 0o755)

generate('orw.elf', asm(
    '''
    mov rdi, [rsp]
    cmp rdi, 2
    jb exit
    mov rdi, [rsp + 16] /* argv[1] */
    {}
    jmp exit
exit:
    xor rdi, rdi
    mov rax, 60
    syscall

    mov rdi, [rsp + 16] /* argv[1] */

    '''.format(
        shellcraft.open('rdi', 0, 0) +
        shellcraft.read('rax', 'rsp', 0x1000) +
        shellcraft.write(1, 'rsp', 'rax')
        )))

