#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"
#include "nanojit.h"
#include "nj_types.h"

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
    case (X):                                                                  \
        return #X;

static const char* nj_inst_name(nj_inst_t inst)
{
    switch (inst) {
        NAME(nj_inst_nop)
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
        NAME(nj_inst_jmp)
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
        return "";
    }
}

static const char* nj_dasm_inst(const char* base, const char* head)
{
    uintptr_t offs = head-base;
    fprintf(stdout, "%04x  ", (long)offs);

    nj_inst_t inst;
    PTR_READ(head, inst);
    const char* name = nj_inst_name(inst);
    fprintf(stdout, "%-16s", name);

    switch (inst) {
    case (nj_inst_const): {
        nj_int_t imm;
        PTR_READ(head, imm);
        fprintf(stdout, "%d", imm);
        break;
    }
    case (nj_inst_arg):
    case (nj_inst_lget):
    case (nj_inst_lset):
    case (nj_inst_debug):
    case (nj_inst_jmp):
    case (nj_inst_call):
    case (nj_inst_frame): {
        nj_uint_t imm;
        PTR_READ(head, imm);
        fprintf(stdout, "%Xh", imm);
        break;
    }
    case (nj_inst_syscall):
    case (nj_inst_ld):
    case (nj_inst_st): {
        assert(!"BLAH");
        break;
    }
    default:
        break;
    }

    fprintf(stdout, "\n");

    return head;
}

void nj_disasm(nj_cxt_t* cxt)
{
    const char* base = buff_data(cxt->code_, 0);
    const char* head = base;
    const char* end = head + buff_size(cxt->code_);

    while (head < end) {
        head = nj_dasm_inst(base, head);
    }
}
