#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"
#include "nanojit.h"
#include "nj_types.h"

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