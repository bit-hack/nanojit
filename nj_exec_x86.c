#if 0
nj_inst_nop
    emit(nop)

nj_inst_const
    emit(push #value)

nj_inst_fp
    emit(push ebp)

nj_inst_arg
    mov eax, [ebp - #index]
    push eax

nj_inst_lget
    mov eax, [ebp + #index]
    push eax

nj_inst_lset 
    pop eax
    mov [ebp + #index], eax

nj_inst_add
    pop eax
    pop ecx
    add eax, ecx
    push eax

nj_inst_sub  
nj_inst_mul
nj_inst_mod
nj_inst_div
nj_inst_shl
nj_inst_shr
nj_inst_sra
nj_inst_and
nj_inst_not
nj_inst_or
nj_inst_xor

nj_inst_dup  
    mov eax, [esp - 4]
    push eax

nj_inst_pop  
    pop eax

nj_inst_jmp
    pop eax
    cmp eax, 0
    jne #dest

nj_inst_lt
    pop eax
    pop ebx
    cmp eax, ebx
    mov edx, 0
    mov ecx, 1
    cmovlt ch, cl
    push ecx

nj_inst_le
nj_inst_eq
nj_inst_ne
nj_inst_ge
nj_inst_gt

nj_inst_ld   
nj_inst_st   

nj_inst_call
    call #function
    push eax

nj_inst_frame
    push ebp
    mov ebp, esp
    sub esp, #frame_size

nj_inst_ret
    pop eax
    add esp, #frame_size
    ret

nj_inst_sys
    // update the nj_execxt_t structure

:nj_inst_lt:nj_inst_jmp:
    pop eax
    pop ebx
    cmp eax, ebx
    jlt LABEL

:nj_const !0:nj_jmp:
    jmp LABEL

:nj_call:nj_ret:
    // make this a tail call somehow

#endif

#include <stdint.h>

enum {
    x86_eax,
    x86_ecx,
    x86_edx,
    x86_ebx,
    x86_esp,
    x86_ebp,
    x86_esi,
    x86_edi
};

static const uint8_t nj_xor__eax_eax[] = { 0x31, 0xc0       };
static const uint8_t nj_xor__ecx_ecx[] = { 0x31, 0xc9       };
static const uint8_t nj_xor__edx_edx[] = { 0x31, 0xd2       };
static const uint8_t nj_xor__ebx_ebx[] = { 0x31, 0xdb       };
static const uint8_t nj_cmp__eax_ebx[] = { 0x39, 0xd8       };
static const uint8_t nj_push_eax    [] = { 0x50             };
static const uint8_t nj_push_ecx    [] = { 0x51             };
static const uint8_t nj_push_edx    [] = { 0x52             };
static const uint8_t nj_push_ebx    [] = { 0x53             };
static const uint8_t nj_push_esp    [] = { 0x54             };
static const uint8_t nj_push_ebp    [] = { 0x55             };
static const uint8_t nj_push_esi    [] = { 0x56             };
static const uint8_t nj_push_edi    [] = { 0x57             };
static const uint8_t nj_pop__eax    [] = { 0x58             };
static const uint8_t nj_pop__ecx    [] = { 0x59             };
static const uint8_t nj_pop__edx    [] = { 0x5a             };
static const uint8_t nj_pop__ebx    [] = { 0x5b             };
static const uint8_t nj_pop__esp    [] = { 0x5c             };
static const uint8_t nj_pop__ebp    [] = { 0x5d             };
static const uint8_t nj_pop__esi    [] = { 0x5e             };
static const uint8_t nj_pop__edi    [] = { 0x5f             };
static const uint8_t nj_cmp__eax_0x0[] = { 0x83, 0xf8, 0x00 };
static const uint8_t nj_add__eax_ebx[] = { 0x01, 0xd8       };
static const uint8_t nj_sub__eax_ebx[] = { 0x29, 0xd8       };
static const uint8_t nj_and__eax_ebx[] = { 0x21, 0xd8       };
static const uint8_t nj_or___eax_ebx[] = { 0x09, 0xd8       };
static const uint8_t nj_xor__eax_ebx[] = { 0x31, 0xd8       };
static const uint8_t nj_not__eax    [] = { 0xf7, 0xd0       };
static const uint8_t nj_call_eax    [] = { 0xff, 0xd0       };
static const uint8_t nj_ret         [] = { 0xc3             };
static const uint8_t nj_nop         [] = { 0x90             };
static const uint8_t nj_mov__ebp_esp[] = { 0x89, 0xe5       };

// 0:  83 ec 10                sub    esp, 0x10
// 0:  81 ec 34 12 00 00       sub    esp, 0x1234

// 0:  83 c4 10                add    esp, 0x10 
// 0:  81 c4 34 12 00 00       add    esp, 0x1234

// 0:  55                      push   ebp
// 1:  89 e5                   mov    ebp, esp 
