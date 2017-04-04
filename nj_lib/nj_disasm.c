#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"
#include "nanojit.h"
#include "nanojit_impl.h"

#define PTR_READ(PTR, VAR)                                                     \
    {                                                                          \
        memcpy(&VAR, PTR, sizeof(VAR));                                        \
        PTR += sizeof(VAR);                                                    \
    }

#define PTR_PEEK(PTR, VAR)                                                     \
    {                                                                          \
        memcpy(&VAR, PTR, sizeof(VAR));                                        \
    }

#define NAME(X)                                                                \
    case (X): {                                                                \
        name = #X;                                                             \
    } break;

const char* nj_inst_name(nj_inst_t inst)
{
    const char* name = NULL;
    switch (inst) {
        NAME(nj_inst_nop)
        NAME(nj_inst_label)
        NAME(nj_inst_const)
        NAME(nj_inst_fp)
        NAME(nj_inst_arg)
        NAME(nj_inst_lget)
        NAME(nj_inst_lset)
        NAME(nj_inst_debug)
        NAME(nj_inst_add)
        NAME(nj_inst_sub)
        NAME(nj_inst_mul)
        NAME(nj_inst_mod)
        NAME(nj_inst_div)
        NAME(nj_inst_shl)
        NAME(nj_inst_shr)
        NAME(nj_inst_sra)
        NAME(nj_inst_and)
        NAME(nj_inst_not)
        NAME(nj_inst_or)
        NAME(nj_inst_xor)
        NAME(nj_inst_dup)
        NAME(nj_inst_pop)
        NAME(nj_inst_cjmp)
        NAME(nj_inst_lt)
        NAME(nj_inst_le)
        NAME(nj_inst_eq)
        NAME(nj_inst_ne)
        NAME(nj_inst_ge)
        NAME(nj_inst_gt)
        NAME(nj_inst_ld)
        NAME(nj_inst_st)
        NAME(nj_inst_call)
        NAME(nj_inst_frame)
        NAME(nj_inst_ret)
        NAME(nj_inst_syscall)
    default:
        break;
    }
    return (name) ? (name + 8) : NULL;
}

void nj_disasm_inst(const char* base, const char* ptr, nj_dasm_t* out)
{
    assert(ptr && out);
    memset(out, 0, sizeof(nj_dasm_t));
    out->addr_ = (nj_uint_t)((uintptr_t)ptr - (uintptr_t)base);
    out->size_ = sizeof(nj_inst_t);
    nj_inst_t inst;
    PTR_READ(ptr, inst);
    out->name_ = nj_inst_name(inst);
    // check for immediates
    switch (inst) {
    case (nj_inst_const): {
        PTR_READ(ptr, out->imms_);
        out->size_ += sizeof(out->imms_);
        out->flag_ = nj_inst_flag_imms_;
        break;
    }
    case (nj_inst_arg):
    case (nj_inst_lget):
    case (nj_inst_lset):
    case (nj_inst_ld):
    case (nj_inst_st):
    case (nj_inst_debug):
    case (nj_inst_cjmp):
    case (nj_inst_call):
    case (nj_inst_ret):
    case (nj_inst_frame): {
        PTR_READ(ptr, out->immu_);
        out->size_ += sizeof(out->immu_);
        out->flag_ = nj_inst_flag_immu_;
        break;
    }
    case (nj_inst_syscall): {
        PTR_READ(ptr, out->immp_);
        out->size_ += sizeof(out->immp_);
        out->flag_ = nj_inst_flag_immp_;
        break;
    }
    default:
        // no immediate value
        break;
    }
}

static const char* nj_dasm_inst_print(const char* base, const char* head)
{
    nj_dasm_t dasm;
    nj_disasm_inst(base, head, &dasm);
    // pretty print this structure
    fprintf(stdout, "%04x  ", (long)dasm.addr_);
    fprintf(stdout, "%-16s  ", dasm.name_);
    switch (dasm.flag_) {
    case (nj_inst_flag_immu_):
        fprintf(stdout, "h%x", dasm.immu_);
        break;
    case (nj_inst_flag_imms_):
        fprintf(stdout, "%d", dasm.imms_);
        break;
    case (nj_inst_flag_immp_):
        fprintf(stdout, "%p", (void*)dasm.immp_);
        break;
    case (nj_inst_flag_none_):
        break;
    }
    fprintf(stdout, "\n");
    // return start of the next instruction
    return head + dasm.size_;
}

static void nj_func_list(nj_cxt_t* cxt)
{
    printf("functions:\n");
    nj_func_t* func = cxt->func_;
    for (; func; func = func->next_) {
        printf("%04x  ", func->start_);
        printf("%s", func->name_);
        printf("\n");
    }
    printf("\n");
}

void nj_disasm(nj_cxt_t* cxt)
{
    const char* base = buff_data(cxt->code_, 0);
    const char* head = base;
    const char* end = head + buff_size(cxt->code_);
    // print a function list
    nj_func_list(cxt);
    // find the next function address
    nj_func_t* func = cxt->func_;
    const char* start = func ? (base + func->start_) : NULL;
    // while there is more to disassemble
    while (head < end) {
        // if a function is at this location
        if (head == start) {
            // print a function header
            printf("%s:\n", func->name_);
            func = func->next_;
            start = func ? (base + func->start_) : NULL;
        }
        // print the instruction
        head = nj_dasm_inst_print(base, head);
    }
}
