#include <stdio.h>
#include "../nj_lib/nanojit.h"
#include "../nj_exe_vm/nj_exe_vm.h"

static int fibonacci(int n)
{
    if (n == 0)
        return 0;
    if (n == 1)
        return 1;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

static void jit_fionacci()
{
    nj_cxt_t* nj = NULL;
    nj = nj_context_create();

    // int factoral( int n )
    nj_func_t* func = NULL;
    func = nj_func_create(nj, "fionacci");
    nj_func_place(func);
    nj_emit_frame(func, 0);

    // ( n == 0 )
    nj_emit_const(func, 0);
    nj_emit_arg(func, 0);
    nj_emit_ne(func);

    // if ( n == 0 ) return 0;
    {
        nj_label_t* label = NULL;
        label = nj_label_create(func);
        nj_emit_cjmp(func, label);
        nj_emit_const(func, 0);
        nj_emit_ret(func, 1);

        nj_label_place(func, label);
    }

    // ( n == 1 )
    nj_emit_const(func, 1);
    nj_emit_arg(func, 0);
    nj_emit_ne(func);

    // if ( n == 1 ) return 1;
    {
        nj_label_t* label = NULL;
        label = nj_label_create(func);
        nj_emit_cjmp(func, label);
        nj_emit_const(func, 1);
        nj_emit_ret(func, 1);

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
    nj_emit_ret(func, 1);

    // generate evereything
    nj_finish(nj);

    // disassemble it
    nj_disasm(nj);

    // execute factoral
    nj_exe_t exe;
    nj_exe_vm_init(nj, &exe);
    nj_int_t arg = 8;
    nj_exe_vm_prepare(&exe, "fionacci", &arg, 1);
    nj_int_t ret = nj_exe_vm_run(&exe);

    printf("fionacci of %d is %d\n", arg, ret);

    nj_context_free(nj);
}

#if 0
int main()
{
    atexit((void(*)(void))getchar);
    jit_fionacci();
    return 0;
}
#endif
