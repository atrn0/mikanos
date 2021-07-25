; asmfunc.asm
;
; System V AMD64 Calling Convention
; Registers: RDI, RSI, RDX, RCX, R8, R9

bits 64
section .text

global IoOut32 ; void IoOut32(uint16_t addr, uint32_t data);
IoOut32:
  mov dx, di   ; dx = addr
  mov eax, esi ; eax = data
  out dx, eax
  ret

global IoIn32 ; uint32_t IoIn32(uint16_t addr);
IoIn32:
  mov dx, di ; dx = addr
  in eax, dx
  ret

global GetCS ; uint16_t GetCS(void);
GetCS:
  xor eax, eax ; also clears upper 32 bits of rax
  mov ax, cs
  ret

global LoadIDT ; void LoadIDT(uint16_t limit, uint64_t offset);
LoadIDT:
  push rbp
  mov rbp, rsp
  sub rsp, 10
  mov [rsp], di ; limit
  mov [rsp + 2], rsi ; offset
  lidt [rsp]
  mov rsp, rbp
  pop rbp
  ret

; Global Descriptor Table
global LoadGDT ; void LoadGDT(uint16_t limit, uint64_t offset);
LoadGDT:
  push rbp
  mov rbp, rsp
  sub rsp, 10
  mov [rsp], di ; limit
  mov [rsp + 2], rsi ; offset
  lgdt [rsp]
  mov rsp, rbp
  pop rbp
  ret

; Code Segment, Stack Segment
global SetCSSS ; void SetCSSS(uint16_t cs, uint16_t ss);
SetCSSS:
  push rbp
  mov rbp, rsp
  mov ss, si
  mov rax, .next
  push rdi ; CS
  push rax ; RIP
  o64 retf ; CSはmovで書き換えることができないので、
           ; far callで呼び出した関数からの復帰に使うretf命令を使う
.next:
    mov rsp, rbp
    pop rbp
    ret

; x86_64の64bitモードでは使われないレジスタ
global SetDSAll  ; void SetDSAll(uint16_t value);
SetDSAll:
    mov ds, di
    mov es, di
    mov fs, di
    mov gs, di
    ret

; CR3 は PML4 テーブルの物理アドレスを保持するレジスタ
global SetCR3  ; void SetCR3(uint64_t value);
SetCR3:
    mov cr3, rdi
    ret

extern kernel_main_stack
extern KernelMainNewStack

; kernelのエントリーポイント
; スタック領域をUEFI管理の物からOS管理の物に移す
global KernelMain
KernelMain:
    mov rsp, kernel_main_stack + 1024 * 1024
    call KernelMainNewStack
.fin:
    hlt
    jmp .fin
