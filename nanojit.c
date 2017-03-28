#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"
#include "nanojit.h"

typedef enum nj_inst_t nj_inst_t;

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

struct nj_label_t {
    nj_label_t* next_;
    uint32_t location_;
};

static inline void nj_emit_opcode(nj_cxt_t* cxt, nj_inst_t opcode)
{
    buff_write(cxt->code_, &opcode, sizeof(opcode));
}

static inline void nj_insert_fixup(nj_cxt_t* cxt, nj_uint_t* src)
{
    nj_fixup_t* fixup = malloc(sizeof(nj_fixup_t));
    assert(fixup);
    memset(fixup, 0, sizeof(nj_fixup_t));
    // insert into the linked list of fixups
    fixup->next_ = cxt->fixup_;
    cxt->fixup_ = fixup;
    // store the fixup information
    fixup->dst_index_ = buff_head(cxt->code_);
    fixup->src_ = src;
    // insert dummy pointer for fixup
    const nj_uint_t zero = 0u;
    buff_write(cxt->code_, &zero, sizeof(zero));
}

nj_cxt_t* nj_context_create()
{
    nj_cxt_t* cxt = (nj_cxt_t*)malloc(sizeof(nj_cxt_t));
    assert(cxt);
    memset(cxt, 0, sizeof(nj_cxt_t));
    cxt->code_ = buff_create(0);
    assert(cxt->code_);
    cxt->data_ = buff_create(0);
    assert(cxt->data_);
    cxt->debug_ = buff_create(0);
    assert(cxt->debug_);
    return cxt;
}

void nj_context_free(nj_cxt_t* cxt)
{
    assert(cxt);
    buff_free(cxt->code_);
    cxt->code_ = NULL;
    buff_free(cxt->data_);
    cxt->data_ = NULL;
    buff_free(cxt->debug_);
    cxt->debug_ = NULL;
    free(cxt);
}

void nj_finish(nj_cxt_t* cxt)
{
    assert(cxt && cxt->code_);
    // apply all fixups
    nj_fixup_t * fixup = cxt->fixup_;
    for (; fixup; fixup = fixup->next_) {
        assert(fixup);
        // seek to the fixup location
        buff_seek(cxt->code_, fixup->dst_index_);
        assert(fixup->src_);
        assert(*fixup->src_!=NJ_VOID_ADDR);
        // insert the fixup we need
        buff_write(cxt->code_, fixup->src_, sizeof(nj_uint_t));
    }
}

nj_uint_t nj_data(nj_cxt_t* cxt, void* data, nj_uint_t size)
{
    assert(cxt && data && size);
    uintptr_t pos = buff_head(cxt->data_);
    buff_write(cxt->data_, data, size);
    return (nj_uint_t)pos;
}

void nj_emit_debug(nj_cxt_t* cxt, const char* data, nj_uint_t size)
{
    assert(cxt && data);
    // add debug data to debug section
    nj_uint_t ptr = buff_head(cxt->code_);
    buff_write(cxt->debug_, cxt->debug_, size);
    // emit debug opcode
    nj_emit_opcode(cxt, nj_inst_debug);
    buff_write(cxt->code_, &ptr, sizeof(ptr));
}

nj_func_t* nj_func_create(nj_cxt_t* cxt, const char* name)
{
    assert(cxt && name);
    nj_func_t* func = malloc(sizeof(nj_func_t));
    assert(func);
    memset(func, 0, sizeof(nj_func_t));
    func->cxt_ = cxt;
    func->name_ = _strdup(name);
    // will be populated when the function is placed
    func->location_ = NJ_VOID_ADDR;
    // insert into the linked list
    func->next_ = cxt->func_;
    cxt->func_ = func;
    // return this function object
    return func;
}

void nj_emit_nop(nj_func_t* func)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    nj_emit_opcode(func->cxt_, nj_inst_nop);
}

void nj_emit_const(nj_func_t* func, nj_int_t value)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    nj_emit_opcode(func->cxt_, nj_inst_const);
    buff_write(func->cxt_->code_, &value, sizeof(value));
}

void nj_emit_fp(nj_func_t* func)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    nj_emit_opcode(func->cxt_, nj_inst_fp);
}

nj_label_t* nj_label_create(nj_func_t* func)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    nj_label_t* label = malloc(sizeof(nj_label_t));
    memset(label, 0, sizeof(nj_label_t));
    label->location_ = NJ_VOID_ADDR;
    // insert into label linked list
    label->next_ = func->label_;
    func->label_ = label;
    return label;
}

void nj_label_place(nj_func_t* func, nj_label_t* label)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    assert(label && label->location_ == NJ_VOID_ADDR);
    label->location_ = buff_head(func->cxt_->code_);
}

void nj_emit_add(nj_func_t* func)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    nj_emit_opcode(func->cxt_, nj_inst_add);
}

void nj_emit_sub(nj_func_t* func)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    nj_emit_opcode(func->cxt_, nj_inst_sub);
}

void nj_emit_mul(nj_func_t* func)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    nj_emit_opcode(func->cxt_, nj_inst_mul);
}

void nj_emit_mod(nj_func_t* func)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    nj_emit_opcode(func->cxt_, nj_inst_mod);
}

void nj_emit_div(nj_func_t* func)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    nj_emit_opcode(func->cxt_, nj_inst_div);
}

void nj_emit_shl(nj_func_t* func)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    nj_emit_opcode(func->cxt_, nj_inst_shl);
}

void nj_emit_shr(nj_func_t* func)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    nj_emit_opcode(func->cxt_, nj_inst_shr);
}

void nj_emit_sra(nj_func_t* func)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    nj_emit_opcode(func->cxt_, nj_inst_sra);
}

void nj_emit_and(nj_func_t* func)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    nj_emit_opcode(func->cxt_, nj_inst_and);
}

void nj_emit_not(nj_func_t* func)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    nj_emit_opcode(func->cxt_, nj_inst_not);
}

void nj_emit_xor(nj_func_t* func)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    nj_emit_opcode(func->cxt_, nj_inst_xor);
}

void nj_emit_or(nj_func_t* func)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    nj_emit_opcode(func->cxt_, nj_inst_or);
}

void nj_emit_dup(nj_func_t* func)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    nj_emit_opcode(func->cxt_, nj_inst_dup);
}

void nj_emit_pop(nj_func_t* func)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    nj_emit_opcode(func->cxt_, nj_inst_pop);
}

void nj_emit_jmp(nj_func_t* func, nj_label_t* label)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    assert(label);
    nj_emit_opcode(func->cxt_, nj_inst_jmp);
    nj_insert_fixup(func->cxt_, &(label->location_));
}

void nj_emit_lt(nj_func_t* func)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    nj_emit_opcode(func->cxt_, nj_inst_lt);
}

void nj_emit_le(nj_func_t* func)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    nj_emit_opcode(func->cxt_, nj_inst_le);
}

void nj_emit_eq(nj_func_t* func)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    nj_emit_opcode(func->cxt_, nj_inst_eq);
}

void nj_emit_ne(nj_func_t* func)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    nj_emit_opcode(func->cxt_, nj_inst_ne);
}

void nj_emit_ge(nj_func_t* func)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    nj_emit_opcode(func->cxt_, nj_inst_ge);
}

void nj_emit_gt(nj_func_t* func)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    nj_emit_opcode(func->cxt_, nj_inst_gt);
}

void nj_emit_ld(nj_func_t* func, nj_uint_t loc)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    nj_emit_opcode(func->cxt_, nj_inst_ld);
    buff_write(func->cxt_->code_, &loc, sizeof(loc));
}

void nj_emit_st(nj_func_t* func, nj_uint_t loc)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    nj_emit_opcode(func->cxt_, nj_inst_st);
    buff_write(func->cxt_->code_, &loc, sizeof(loc));
}

void nj_emit_call(nj_func_t* func, nj_func_t* callee)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    assert(callee);
    nj_emit_opcode(func->cxt_, nj_inst_call);
    nj_insert_fixup(func->cxt_, &(callee->location_));
}

void nj_emit_frame(nj_func_t* func, nj_uint_t size)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    nj_emit_opcode(func->cxt_, nj_inst_frame);
    buff_write(func->cxt_->code_, &size, sizeof(size));
}

void nj_emit_ret(nj_func_t* func)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    nj_emit_opcode(func->cxt_, nj_inst_ret);
}

void nj_emit_sys(nj_func_t* func, nj_syscall_t sys)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    assert(sys);
    nj_emit_opcode(func->cxt_, nj_inst_syscall);
    // TODO: abstract this since its host size dependant
    buff_write(func->cxt_->code_, &sys, sizeof(sys));
}

void nj_emit_arg(nj_func_t* func, nj_uint_t index)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    nj_emit_opcode(func->cxt_, nj_inst_arg);
    buff_write(func->cxt_->code_, &index, sizeof(index));
}

void nj_emit_lget(nj_func_t* func, nj_uint_t index)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    nj_emit_opcode(func->cxt_, nj_inst_lget);
    buff_write(func->cxt_->code_, &index, sizeof(index));
}

void nj_emit_lset(nj_func_t* func, nj_uint_t index)
{
    assert(func && func->cxt_ && (func->location_ != NJ_VOID_ADDR));
    nj_emit_opcode(func->cxt_, nj_inst_lset);
    buff_write(func->cxt_->code_, &index, sizeof(index));
}
