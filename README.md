# yaye
yaye is Yet Another Y68 Emulator

 * Uses GCC.

 * The max. memory size is set by a macro (MAX_MEM) in yaye.h, the default is 4096 bytes.


Please build with:
    make

Then run with:
    ./yaye <filename>

# Sample Program
 * Also included is a y86 program that exploits a stack buffer overflow. The program file is called "bufferoverflow" and the readme with the corresponding reference assembly code is in "bufferoverflow_readme.txt".
