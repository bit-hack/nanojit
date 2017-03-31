#include <stdio.h>
#include "../nj_lib/nanojit.h"

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
