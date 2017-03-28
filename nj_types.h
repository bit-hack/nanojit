#pragma once
#include "nanojit.h"

typedef enum nj_inst_t {
    nj_inst_nop, // no operation
    nj_inst_const, // push constant onto the stack
    nj_inst_fp, // push frame pointer onto the stack
    nj_inst_arg, // extract argument using fp
    nj_inst_lget, // read local using fp
    nj_inst_lset, // write local using fp
    nj_inst_debug, // insert debug data
    nj_inst_add,
    nj_inst_sub, // stk[-1] - stk[0]
    nj_inst_mul,
    nj_inst_mod,
    nj_inst_div,
    nj_inst_shl,
    nj_inst_shr,
    nj_inst_sra,
    nj_inst_and,
    nj_inst_not,
    nj_inst_or,
    nj_inst_xor,
    nj_inst_dup, // duplicate top stack item
    nj_inst_pop, // discard top stack item
    nj_inst_jmp,
    nj_inst_lt, // stk[-1] < stk[0]
    nj_inst_le,
    nj_inst_eq,
    nj_inst_ne,
    nj_inst_ge,
    nj_inst_gt,
    nj_inst_ld, // memory load
    nj_inst_st, // memory store
    nj_inst_call, // function call
    nj_inst_frame, // reserve some local stack space
    nj_inst_ret, // destroy stack frame preserving top most item
    nj_inst_syscall, // syscall
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
    uint32_t location_;
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
    uint32_t location_;
};
typedef struct nj_label_t nj_label_t;

#define NJ_VOID_ADDR (~(nj_uint_t)0u)
