
minimal.o：     文件格式 elf64-x86-64


Disassembly of section .text:

0000000000000000 <_start>:
   0:	48 c7 c0 01 00 00 00 	mov    $0x1,%rax
   7:	48 c7 c7 01 00 00 00 	mov    $0x1,%rdi
   e:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
  15:	48 c7 c2 1c 00 00 00 	mov    $0x1c,%rdx
  1c:	0f 05                	syscall 
  1e:	48 c7 c0 3c 00 00 00 	mov    $0x3c,%rax
  25:	48 c7 c7 01 00 00 00 	mov    $0x1,%rdi
  2c:	0f 05                	syscall 

000000000000002e <st>:
  2e:	1b 5b 30             	sbb    0x30(%rbx),%ebx
  31:	31 3b                	xor    %edi,(%rbx)
  33:	33 31                	xor    (%rcx),%esi
  35:	6d                   	insl   (%dx),%es:(%rdi)
  36:	48                   	rex.W
  37:	65 6c                	gs insb (%dx),%es:(%rdi)
  39:	6c                   	insb   (%dx),%es:(%rdi)
  3a:	6f                   	outsl  %ds:(%rsi),(%dx)
  3b:	2c 20                	sub    $0x20,%al
  3d:	4f 53                	rex.WRXB push %r11
  3f:	20 57 6f             	and    %dl,0x6f(%rdi)
  42:	72 6c                	jb     b0 <ed+0x66>
  44:	64 1b 5b 30          	sbb    %fs:0x30(%rbx),%ebx
  48:	6d                   	insl   (%dx),%es:(%rdi)
  49:	0a                   	.byte 0xa
