# KVM-Kernel Example

The source code are examples on my blog: [Learning KVM - implement your own Linux kernel](https://david942j.blogspot.com/2018/10/note-learning-kvm-implement-your-own.html).

I've described how to implement a KVM-based hypervisor and key points to implement Linux kernel on my blog.
You can leave comments in the blog or file issues here if you have questions or find any bugs.

## Dir

### Hypervisor

The KVM-based hypervisor, its role is like qemu-system.

### Kernel

A extremely simple Linux kernel, supports few syscalls.

### User

Simple ELF(s) for testing our kernel.
Pre-built user program was provided, and you can re-generate by the following commands:
```sh
$ pip2 install pwn
$ cd user
$ ./gen.py
```
NOTE: You have to install Python 2.x in advance.

### How to run

```sh
$ git clone https://github.com/david942j/kvm-kernel-example
$ cd kvm-kernel-example && make
$ sudo hypervisor/hypervisor.elf kernel/kernel.bin user/orw.elf /etc/os-release
# NAME="Ubuntu"
# VERSION="18.04.1 LTS (Bionic Beaver)"
# ID=ubuntu
# ID_LIKE=debian
# PRETTY_NAME="Ubuntu 18.04.1 LTS"
# VERSION_ID="18.04"
# HOME_URL="https://www.ubuntu.com/"
# SUPPORT_URL="https://help.ubuntu.com/"
# BUG_REPORT_URL="https://bugs.launchpad.net/ubuntu/"
# PRIVACY_POLICY_URL="https://www.ubuntu.com/legal/terms-and-policies/privacy-policy"
# VERSION_CODENAME=bionic
# UBUNTU_CODENAME=bionic
# +++ exited with 0 +++
```

### Environment

I only tested the code on Ubuntu 18.04, but I expected it works on all KVM-supported x86 Linux distributions. File an issue if you find it's not true.
