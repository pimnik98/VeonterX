#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <kernel/sys/ports.h>
#include <kernel/sys/gdt.h>
#include <kernel/sys/idt.h>
#include <kernel/sys/isr.h>
#include <kernel/sys/pic.h>
#include <kernel/panic.h>
#include <kernel/sys/pit.h>
#include <kernel/sys/kheap.h>
#include <kernel/sys/paging.h>
#include <kernel/sys/console.h>
#include <kernel/drv/tty.h>
#include <kernel/sys/cpuid.h>
#include <kernel/drv/vbe.h>
#include <kernel/sys/gui/gui.h>
#include <kernel/sys/gui/geometry.h>
#include <kernel/sys/gui/psf.h>
#include <kernel/sys/gui/shell.h>
#include <kernel/drv/keyboard.h>
#include <kernel/drv/speaker.h>
#include <kernel/sys/vga.h>
#include <kernel/multiboot.h>
#include <kernel/hal.h>
#include <kernel/sys/fs/vfs.h>