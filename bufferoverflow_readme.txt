###########################################################################################
# ZSisco
# bufferoverflow_readme.txt
###########################################################################################
# This program calls a routine that 'innocently' pushes a few too many values on the stack.
# The values that are being pushed are actually opcodes for instructions that will
#  overwrite the values of all of the registers with the value: 0x13371337. 
#
# Since the stack is allocated too low the program counter will run into the top of the
#  stack and take a "nop slide" right into the part of memory we loaded our opcodes in.
###########################################################################################


   OPCODE             MNEMONIC INSTRUCTION
------------  -----------------------------------------
30f400010000  irmovl  0x100, %esp   # Set up stack pointer
30f500010000  irmovl  0x100, %ebp   # Set up base pointer
30f1c8000000  irmovl   0xc8, %ecx   # Set %ecx to something non-zero
801a000000    call     func         
6311          xorl     %ecx, %ecx   # Zero-out %ecx; this will never get called                       
00            halt

              func:
a05f          pushl   %ebp
2045          rrmovl  %esp, %ebp
30f030f03713  irmovl  0x1337f030, %eax  # Load opcode values into %eax
30f337131010  irmovl  0x10101337, %ebx  # Load opcode values into %ebx
30f210101010  irmovl  0x10101010, %edx  # Load nop opcodes into %edx
a03f          pushl   %ebx              
a00f          pushl   %eax
30f030f13713  irmovl  0x1337f030, %eax  # Repeat for other opcodes...
30f337131010  irmovl  0x10101337, %ebx
a03f          pushl   %ebx
a00f          pushl   %eax
30f030f23713  irmovl  0x1337f030, %eax
30f337131010  irmovl  0x10101337, %ebx
a03f          pushl   %ebx
a00f          pushl   %eax
30f030f33713  irmovl  0x1337f030, %eax
30f337131010  irmovl  0x10101337, %ebx
a03f          pushl   %ebx
a00f          pushl   %eax
30f030f63713  irmovl  0x1337f030, %eax
30f337131010  irmovl  0x10101337, %ebx
a03f          pushl   %ebx
a00f          pushl   %eax
30f030f73713  irmovl  0x1337f030, %eax
30f337131010  irmovl  0x10101337, %ebx
a03f          pushl   %ebx
a00f          pushl   %eax
30f030f43713  irmovl  0x1337f030, %eax
30f337131010  irmovl  0x10101337, %ebx
a03f          pushl   %ebx
a00f          pushl   %eax
30f030f53713  irmovl  0x1337f030, %eax
30f337131010  irmovl  0x10101337, %ebx
a03f          pushl   %ebx
a00f          pushl   %eax
a02f          pushl   %edx
a02f          pushl   %edx
a02f          pushl   %edx
a02f          pushl   %edx
90            ret            
