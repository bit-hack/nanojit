#include <stdio.h>

#include "buffer.h"
#include "nanojit.h"

#if 0
/*
 * 0000  nj_inst_const   0
 * 0008  nj_inst_arg     0h
 * 0010  nj_inst_eq
 * 0014  nj_inst_jmp     28h
 * 001c  nj_inst_const   0
 * 0024  nj_inst_ret
 * 0028  nj_inst_const   0
 * 0030  nj_inst_arg     0h
 * 0038  nj_inst_eq
 * 003c  nj_inst_jmp     50h
 * 0044  nj_inst_const   1
 * 004c  nj_inst_ret
 * 0050  nj_inst_arg     0h
 * 0058  nj_inst_const   2
 * 0060  nj_inst_sub
 * 0064  nj_inst_call    0h
 * 006c  nj_inst_arg     0h
 * 0074  nj_inst_const   1
 * 007c  nj_inst_sub
 * 0080  nj_inst_call    0h
 * 0088  nj_inst_add
 * 008c  nj_inst_ret
 */
#endif

static int fibonacci(int n)
{
    if (n == 0)
        return 0;
    if (n == 1)
        return 1;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

void jit_factoral()
{
    nj_cxt_t* nj = NULL;
    nj = nj_context_create();

    // int factoral( int n )
    nj_func_t* func = NULL;
    func = nj_func_create(nj, "factoral");
    nj_func_place(func);

    // ( n == 0 )
    nj_emit_const(func, 0);
    nj_emit_arg(func, 0);
    nj_emit_eq(func);

    // if ( n == 0 ) return 0;
    {
        nj_label_t* label = NULL;
        label = nj_label_create(func);
        nj_emit_jmp(func, label);
        nj_emit_const(func, 0);
        nj_emit_ret(func);

        nj_label_place(func, label);
    }

    // ( n == 1 )
    nj_emit_const(func, 0);
    nj_emit_arg(func, 0);
    nj_emit_eq(func);

    // if ( n == 1 ) return 1;
    {
        nj_label_t* label = NULL;
        label = nj_label_create(func);
        nj_emit_jmp(func, label);
        nj_emit_const(func, 1);
        nj_emit_ret(func);

        nj_label_place(func, label);
    }

    // factoral( n-2 )
    nj_emit_arg(func, 0);
    nj_emit_const(func, 2);
    nj_emit_sub(func);
    nj_emit_call(func, func);

    // factoral( n-1 )
    nj_emit_arg(func, 0);
    nj_emit_const(func, 1);
    nj_emit_sub(func);
    nj_emit_call(func, func);

    // return factoral( n-1 ) + factoral( n-2 )
    nj_emit_add(func);
    nj_emit_ret(func);

    // generate evereything
    nj_finish(nj);

    // disassemble it
    nj_disasm(nj);

    nj_context_free(nj);
}

int main()
{
    atexit((void(*)(void))getchar);
    jit_factoral();
    return 0;
}
