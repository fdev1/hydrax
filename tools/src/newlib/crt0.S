.global _start
.extern main
.extern exit
_start:
	call main;
	call exit;
1:	jmp 1b;

