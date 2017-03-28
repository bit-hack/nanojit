#include "buffer.h"
#include "nanojit.h"

int fibonacci(int n)
{
    if (n == 0)
        return 0;
    if (n == 1)
        return 1;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

/**
fibonacci:
    const   0
    arg     0
    ne
    jmp     L1
    const   0
    ret
.L1
    const   1
    arg     0
    ne
    jmp     L2
    const   1
    ret
.L2
    arg     0
    const   2
    sub
    call    "fibonacci"
    arg     0
    const   1
    sub
    call    "fibonacci"
    add
    ret
**/

void jit_factoral()
{
    nj_cxt_t* nj = NULL;
    nj = nj_context_create();

    // int factoral( int n )
    nj_func_t* func = NULL;
    func = nj_func_create(nj, "factoral");

    // ( n == 0 )
    nj_emit_const(func, 0);
    nj_emit_arg(func, 0);
    nj_emit_eq(func);

    // if ( __above__ ) return 0;
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

    nj_context_free(nj);
}

void test_buffer()
{
    buffer_t* buf = buff_create(128);
    buff_write(buf, "Hello World", 11);
    buff_write(buf, "test", 4);
    buff_free(buf);
}

int main()
{
    test_buffer();
    jit_factoral();
    return 0;
}
