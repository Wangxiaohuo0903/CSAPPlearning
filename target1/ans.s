/*
unix> gcc -c example.s
unix> objdump -d example.o > example.d
0x59b997fa->35 39 62 39 39 37 66 61 00
*/
pop %rbx
mov %rbx,%rdi
retq
