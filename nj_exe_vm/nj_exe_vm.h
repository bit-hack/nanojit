#pragma once
#include "../nj_lib/nanojit.h"

void nj_exe_vm_init(nj_cxt_t* cxt, nj_exe_t* out);

void nj_exe_vm_prepare(
    nj_exe_t* exe, const char* name, nj_int_t* args, nj_uint_t argc);

nj_int_t nj_exe_vm_run(nj_exe_t* exe);
