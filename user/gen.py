#!/usr/bin/env python3

from pathlib import Path
from pwn import asm, context, make_elf, shellcraft
import os

def generate(filename, data):
    elf = make_elf(data)
    path = os.path.join(Path().absolute(), filename)
    open(path, 'wb').write(elf)
    os.chmod(path, 0o755)

context.arch = 'amd64'

generate('orw.elf', asm(
    f'''
    mov rdi, [rsp]
    cmp rdi, 2
    jb exit
    mov rdi, [rsp + 16] /* argv[1] */
    {shellcraft.open('rdi', 'O_RDONLY', 0)}
    {shellcraft.read('rax', 'rsp', 0x1000)}
    {shellcraft.write(1, 'rsp', 'rax')}
exit:
    xor rdi, rdi
    mov rax, 60
    syscall

    /* should not reach here */
    hlt
    '''))
