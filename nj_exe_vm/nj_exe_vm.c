#include "../nj_lib/nanojit_impl.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static nj_func_t* nj_find_func(nj_cxt_t* cxt, const char* name)
{
    assert(cxt && name);
    nj_func_t* func = cxt->func_;
    for (; func; func = func->next_) {
        if (strcmp(name, func->name_) == 0) {
            return func;
        }
    }
    return NULL;
}

static void nj_exe_push(nj_exe_t* exe, nj_int_t val)
{
    assert(exe->sp_<NJ_STACK_SIZE);
    exe->stack[exe->sp_] = val;
    ++exe->sp_;
}

static nj_int_t nj_exe_pop(nj_exe_t* exe)
{
    assert(exe->sp_>=exe->fp_);
    --exe->sp_;
    return exe->stack[exe->sp_];
}

static nj_int_t nj_exe_peek(nj_exe_t* exe)
{
    const nj_uint_t index = exe->sp_ - 1u;
    return exe->stack[index];
}

static nj_inst_t nj_read_opcode(const uint8_t** pc)
{
    const nj_inst_t val = *((nj_inst_t*)(*pc));
    (*pc) += sizeof(val);
    return val;
}

static nj_int_t nj_read_imm(const uint8_t** pc)
{
    const nj_int_t val = *((nj_int_t*)(*pc));
    (*pc) += sizeof(val);
    return val;
}

static nj_ptr_t nj_read_ptr(const uint8_t** pc)
{
    const nj_ptr_t val = *((nj_ptr_t*)(*pc));
    (*pc) += sizeof(val);
    return val;
}

void nj_exe_vm_init(nj_cxt_t* cxt, nj_exe_t* out)
{
    assert(cxt && out);
    out->pc_ = NULL;
    out->fp_ = 0;
    out->sp_ = 0;
    out->cxt_ = cxt;
    memset(out->stack, 0, sizeof(out->stack));
}

void nj_exe_vm_prepare(
    nj_exe_t* exe, const char* name, nj_int_t* args, nj_uint_t argc)
{
    assert(exe && name);
    if (argc) {
        assert(args);
    }
    nj_cxt_t* cxt = exe->cxt_;
    assert(cxt);
    nj_func_t* func = nj_find_func(cxt, name);
    assert(func);
    // set PC to function start
    exe->pc_ = buff_data(cxt->code_, func->start_);
    // push any arguments
    for (nj_uint_t i = 0; i < argc; ++i) {
        nj_exe_push(exe, args[i]);
    }
    // push dummy return address
    nj_exe_push(exe, NJ_VOID_ADDR);
}

#define NJ_OP(ENUM, exe, OP)                                                   \
    case (ENUM): {                                                             \
        const nj_int_t a = nj_exe_pop(exe);                                    \
        const nj_int_t b = nj_exe_pop(exe);                                    \
        nj_exe_push(exe, b OP a);                                              \
    } break;

#define TODO()                                                                 \
    {                                                                          \
        assert(!"TODO");                                                       \
    }

nj_int_t nj_exe_vm_run(nj_exe_t* exe)
{
    assert(exe && exe->cxt_);
    nj_cxt_t* cxt = exe->cxt_;
    const uint8_t* pc = exe->pc_;
    for (;;) {
        assert(pc);
        // read instruction opcode
        const uint8_t* new_pc = pc;
        const nj_inst_t opcode = nj_read_opcode(&new_pc);
        // print a simple trace
        printf("-> %s\n", nj_inst_name(opcode));
        // dispatch to opcode handler
        switch (opcode) {
        case (nj_inst_nop):
        case (nj_inst_label): {
            /* no operation instructions */
        } break;
        case (nj_inst_const): {
            nj_int_t val = nj_read_imm(&new_pc);
            nj_exe_push(exe, val);
            break;
        }
        case (nj_inst_fp): {
            nj_exe_push(exe, exe->fp_);
        } break;
        case (nj_inst_arg): {
            const nj_uint_t imm = nj_read_imm(&new_pc);
            const nj_uint_t index = exe->fp_ - (3 + imm);
            const nj_int_t arg = exe->stack[index];
            nj_exe_push(exe, arg);
        } break;
        case (nj_inst_lget): {
            const nj_uint_t imm = nj_read_imm(&new_pc);
            const nj_uint_t index = exe->fp_ + imm;
            const nj_int_t val = exe->stack[index];
            nj_exe_push(exe, val);
        } break;
        case (nj_inst_lset): {
            const nj_uint_t imm = nj_read_imm(&new_pc);
            const nj_uint_t index = exe->fp_ + imm;
            exe->stack[index] = nj_exe_pop(exe);
        } break;
        case (nj_inst_ret): {
            // number of arguments to discard
            const nj_uint_t imm = nj_read_imm(&new_pc);
            // pop (temp) a return value
            const nj_int_t ret_val = nj_exe_pop(exe);
            // discard stack frame
            exe->sp_ = exe->fp_;
            exe->fp_ = nj_exe_pop(exe);
            // pop return address
            const nj_int_t addr = nj_exe_pop(exe);
            if (addr == NJ_VOID_ADDR) {
                // end of execution
                exe->pc_ = NULL;
                return ret_val;
            }
            new_pc = buff_data(cxt->code_, addr);
            // discard arguments passed to this func
            assert(imm < exe->sp_);
            exe->sp_ -= imm;
            // push the return value
            nj_exe_push(exe, ret_val);
        } break;
            NJ_OP(nj_inst_add, exe, +);
            NJ_OP(nj_inst_sub, exe, -);
            NJ_OP(nj_inst_mul, exe, *);
            NJ_OP(nj_inst_mod, exe, %);
            NJ_OP(nj_inst_div, exe, /);
            NJ_OP(nj_inst_shl, exe, <<);
        case (nj_inst_shr): {
            const nj_int_t a = nj_exe_pop(exe);
            const nj_int_t b = nj_exe_pop(exe);
            nj_exe_push(exe, (nj_uint_t)b << (nj_uint_t)a);
            break;
        } break;
            NJ_OP(nj_inst_sra, exe, >>);
            NJ_OP(nj_inst_and, exe, &);
        case (nj_inst_not): {
            const nj_int_t val = nj_exe_pop(exe);
            nj_exe_push(exe, ~val);
        } break;
            NJ_OP(nj_inst_or, exe, |);
            NJ_OP(nj_inst_xor, exe, ^);
        case (nj_inst_dup): {
            const nj_int_t val = nj_exe_peek(exe);
            nj_exe_push(exe, val);
        } break;
        case (nj_inst_pop): {
            assert(exe->sp_ > 0);
            --exe->sp_;
        } break;
            NJ_OP(nj_inst_lt, exe, <);
            NJ_OP(nj_inst_le, exe, <=);
            NJ_OP(nj_inst_eq, exe, ==);
        case (nj_inst_ne): {
            const nj_int_t a = nj_exe_pop(exe);
            const nj_int_t b = nj_exe_pop(exe);
            nj_exe_push(exe, b != a);
        } break;
            NJ_OP(nj_inst_ge, exe, >=);
            NJ_OP(nj_inst_gt, exe, >);
        case (nj_inst_ld): {
            TODO(); /* load from memory */
        } break;
        case (nj_inst_st): {
            TODO(); /* store to memory */
        } break;
        case (nj_inst_cjmp): {
            const nj_int_t imm = nj_read_imm(&new_pc);
            const nj_int_t cnd = nj_exe_pop(exe);
            if (cnd) {
                new_pc = buff_data(cxt->code_, imm);
            }
        } break;
        case (nj_inst_call): {
            // read branch target
            const nj_int_t imm = nj_read_imm(&new_pc);
            assert(imm != NJ_VOID_ADDR);
            // push return address
            const uintptr_t base = (uintptr_t)buff_data(cxt->code_, 0);
            const nj_uint_t ret = (nj_uint_t)((uintptr_t)new_pc - base);
            nj_exe_push(exe, ret);
            // branch to new target
            new_pc = buff_data(cxt->code_, imm);
        } break;
        case (nj_inst_frame): {
            // store old frame pointer
            nj_exe_push(exe, exe->fp_);
            // new frame at stack pos
            exe->fp_ = exe->sp_;
            // allocate space for locals
            const nj_int_t imm = nj_read_imm(&new_pc);
            exe->sp_ += imm;
        } break;
        case (nj_inst_syscall): {
            const nj_ptr_t ptr = nj_read_ptr(&new_pc);
            exe->pc_ = new_pc;
            nj_syscall_t scall = (nj_syscall_t)ptr;
            assert(scall);
            scall(exe);
            new_pc = exe->pc_;
        } break;
        default:
            assert(!"Unknown Opcode");
        }
        pc = new_pc;
    }
    return 0;
}
