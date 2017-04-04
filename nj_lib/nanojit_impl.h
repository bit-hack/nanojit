#pragma once
#include "nanojit.h"
#include "buffer.h"

typedef enum nj_inst_t {
    nj_inst_unknown,
    /* no operation */
    nj_inst_nop,
    /* marks a branch target. acts as a barrier for maximal munch codegen. */
    nj_inst_label,
    /* insert pointer to debug data */
    nj_inst_debug,
    /* push constant onto the stack. */
    nj_inst_const,
    /* push current frame pointer onto the stack. */
    nj_inst_fp,
    /* extract rgument using current FP. */
    nj_inst_arg,
    nj_inst_lget, // read local using fp
    nj_inst_lset, // write local using fp

    nj_inst_add,
    nj_inst_sub, // stk[-1] - stk[0]
    nj_inst_mul,
    nj_inst_mod,
    nj_inst_div,
    nj_inst_shl,
    nj_inst_shr,
    nj_inst_sra,
    nj_inst_and,
    nj_inst_xor,
    nj_inst_or,

    nj_inst_not, // bitwize not
    nj_inst_lnot, // logical not

    nj_inst_dup, // duplicate top stack item
    nj_inst_pop, // discard items from stack

    nj_inst_lt, // stk[-1] < stk[0]
    nj_inst_le,
    nj_inst_eq,
    nj_inst_ne,
    nj_inst_ge,
    nj_inst_gt,

    /* load nj_int_t from memory */
    nj_inst_ld,
    /* store nj_int_t to memory */
    nj_inst_st,

    /* unconditional jump */
    nj_inst_jmp,
    /* conditional jump */
    nj_inst_cjmp,
    nj_inst_call,
    /* deallocate stack frame and preserve top most stack element */
    nj_inst_ret,

    /* allocate stack frame and stack space */
    nj_inst_frame,

    /* execute a system call */
    nj_inst_syscall,
} nj_inst_t;
typedef enum nj_inst_t nj_inst_t;

typedef struct buffer_t buffer_t;

struct nj_fixup_t {
    uint32_t dst_index_;
    uint32_t* src_;
    struct nj_fixup_t* next_;
};
typedef struct nj_fixup_t nj_fixup_t;

struct nj_func_t {
    // function name
    const char* name_;
    // function location in the code section
    uint32_t start_;
//    uint32_t end_;
    // next function linked list
    nj_func_t* next_;
    // the owning context
    nj_cxt_t* cxt_;
    //
    nj_label_t* label_;
};
typedef struct nj_func nj_func;

struct nj_cxt_t {
    // list of functions
    nj_func_t* func_;
    // sections
    buffer_t* code_;
    buffer_t* data_;
    buffer_t* debug_;
    // pending fixups
    struct nj_fixup_t* fixup_;
};
typedef struct nj_cxt_t nj_cxt_t;

struct nj_label_t {
    nj_label_t* next_;
    uint32_t start_;
};
typedef struct nj_label_t nj_label_t;

enum nj_inst_flag_t {
    nj_inst_flag_none_ = 0x0,
    nj_inst_flag_immu_ = 0x1,
    nj_inst_flag_imms_ = 0x2,
    nj_inst_flag_immp_ = 0x4
};
typedef enum nj_inst_flag_t nj_inst_flag_t;

struct nj_dasm_t {
    nj_inst_t inst_;
    nj_uint_t addr_;
    nj_uint_t size_;
    const char* name_;
    nj_inst_flag_t flag_;
    union {
        nj_uint_t immu_;
        nj_int_t imms_;
        nj_ptr_t immp_;
    };
};
typedef struct nj_dasm_t nj_dasm_t;

#define NJ_VOID_ADDR (~(nj_uint_t)0u)

// decompose a single instruction
void nj_disasm_inst(const char* base, const char* ptr, nj_dasm_t* out);

// return the size of a compiled function
size_t nj_func_size(nj_cxt_t * cxt, nj_func_t * func);

// return string name for a given instruction enumeration
const char * nj_inst_name(nj_inst_t inst);
