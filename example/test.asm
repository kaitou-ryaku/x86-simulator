bits 32
org 0x0

start:
  mov eax, 0x0
  mov ecx, 0xa
  jmp cond

loop:
  inc eax
  add ebx, eax

cond:
  cmp eax, ecx
  jl loop

hlt
