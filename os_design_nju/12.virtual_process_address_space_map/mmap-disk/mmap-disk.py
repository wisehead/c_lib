#!/usr/bin/env python3

import mmap, hexdump

#with open('/dev/sda', 'rb') as fp:
with open('/home/chenhui/gcc.version.txt', 'rb') as fp:
    #mm = mmap.mmap(fp.fileno(), prot=mmap.PROT_READ, length=128 << 30)
    mm = mmap.mmap(fp.fileno(), prot=mmap.PROT_READ, length=128)
    hexdump.hexdump(mm[:512])
