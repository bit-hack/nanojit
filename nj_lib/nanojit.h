#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef struct nj_cxt_t nj_cxt_t;
typedef struct nj_label_t nj_label_t;
typedef struct nj_func_t nj_func_t;
typedef struct nj_exe_t nj_exe_t;

typedef void (*nj_syscall_t)(nj_exe_t* exe);

typedef int32_t nj_int_t;
typedef uint32_t nj_uint_t;
typedef uintptr_t nj_ptr_t;

struct nj_exe_t {
#define NJ_STACK_SIZE 1024
    nj_int_t stack[NJ_STACK_SIZE];
    nj_uint_t sp_; // top of stack
    nj_uint_t fp_; // current stack frame
    const uint8_t* pc_; // program counter
    nj_cxt_t* cxt_; // nanojit context
};

// dissassemble nano jit context
void nj_disasm(nj_cxt_t* cxt);

// create a nanojit context
nj_cxt_t* nj_context_create();

// release a nanojit context
void nj_context_free(nj_cxt_t* cxt);

// finalize and verify the context
void nj_finish(nj_cxt_t* cxt);

// place data in the data section
uint32_t nj_data(nj_cxt_t* cxt, void* data, nj_uint_t size);

// emit debug info
void nj_emit_debug(nj_cxt_t* cxt, const char* data, nj_uint_t size);

// create a function
nj_func_t* nj_func_create(nj_cxt_t* cxt, const char* name);
void nj_func_place(nj_func_t* func);
bool nj_func_is_placed(nj_func_t* func);

// no operation instruction
void nj_emit_nop(nj_func_t* func);

// push constant value onto the stack
void nj_emit_const(nj_func_t* func, nj_int_t value);

// push frame pointer onto the stack
void nj_emit_fp(nj_func_t* func);

// emit a jump target
nj_label_t* nj_label_create(nj_func_t* func);
void nj_label_place(nj_func_t* func, nj_label_t* label);
bool nj_label_is_placed(nj_label_t* label);

// arithmetic
void nj_emit_add(nj_func_t* func);
void nj_emit_sub(nj_func_t* func);
void nj_emit_mul(nj_func_t* func);
void nj_emit_mod(nj_func_t* func);
void nj_emit_div(nj_func_t* func);
void nj_emit_shl(nj_func_t* func);
void nj_emit_shr(nj_func_t* func);
void nj_emit_sra(nj_func_t* func);
void nj_emit_and(nj_func_t* func);
void nj_emit_not(nj_func_t* func);
void nj_emit_xor(nj_func_t* func);
void nj_emit_or(nj_func_t* func);
void nj_emit_lnot(nj_func_t* func);

// duplicate top stack item
void nj_emit_dup(nj_func_t* func);

// discard top stack item
void nj_emit_pop(nj_func_t* func, nj_uint_t num);

// unconditional jump to label
void nj_emit_jmp(nj_func_t* func, nj_label_t* label);
// conditional jump to a label
void nj_emit_cjmp(nj_func_t* func, nj_label_t* label);

// comparison
void nj_emit_lt(nj_func_t* func);
void nj_emit_le(nj_func_t* func);
void nj_emit_eq(nj_func_t* func);
void nj_emit_ne(nj_func_t* func);
void nj_emit_ge(nj_func_t* func);
void nj_emit_gt(nj_func_t* func);

// memory access
void nj_emit_ld(nj_func_t* func, nj_uint_t);
void nj_emit_st(nj_func_t* func, nj_uint_t);

// call a function
void nj_emit_call(nj_func_t* func, nj_func_t* callee);

// create a stack frame
void nj_emit_frame(nj_func_t* func, nj_uint_t size);

// emit function return
void nj_emit_ret(nj_func_t* func, nj_uint_t nargs);

// emit syscall
void nj_emit_syscall(nj_func_t* func, nj_syscall_t sys);

// emit argument lookup
void nj_emit_arg(nj_func_t* func, nj_uint_t index);

// emit local get/set
void nj_emit_lget(nj_func_t* func, nj_uint_t index);
void nj_emit_lset(nj_func_t* func, nj_uint_t index);
