#include <assert.h>
#include <stdio.h>

#include "../nj_lib/buffer.h"
#include "../nj_lib/nanojit_impl.h"
#include "parser.h"

struct def_func_t {
    struct def_func_t* next_;
    char* name_;
    nj_func_t* func_;
};
typedef struct def_func_t def_func_t;

struct def_label_t {
    struct def_label_t* next_;
    char* name_;
    nj_label_t* label_;
};
typedef struct def_label_t def_label_t;

struct state_t {
    bool error_;
    parser_t parser_;
    nj_cxt_t* cxt_;
    nj_func_t* func_;
    def_func_t* func_list_;
    def_label_t* label_list_;
};
typedef struct state_t state_t;

static bool compare(const char* a, const char* b)
{
    for (;;) {
        if (*a == '\0' || *b == '\0') {
            return true;
        }
        if (*a != *b) {
            return false;
        }
        ++a, ++b;
    }
}

#define MATCH(STR, ENUM)                                                       \
    if (compare(token->start_, STR)) {                                         \
        return (ENUM);                                                         \
    }

static void next_token(state_t* s, token_t* tok)
{
    assert(s && tok);
    token_clear(tok);
    parse_next(&s->parser_, tok);
}

static def_func_t* find_def_func(state_t* s, token_t* t)
{
    assert(s && t);
    def_func_t* def = s->func_list_;
    for (; def; def = def->next_) {
        if (token_match(t, def->name_)) {
            return def;
        }
    }
    return NULL;
}

static def_label_t* find_def_label(state_t* s, token_t* t)
{
    assert(s && t);
    def_label_t* def = s->label_list_;
    for (; def; def = def->next_) {
        if (token_match(t, def->name_)) {
            return def;
        }
    }
    return NULL;
}

static def_func_t* new_def_func(state_t* s)
{
    def_func_t* def = malloc(sizeof(def_func_t));
    assert(def);
    memset(def, 0, sizeof(def_func_t));
    def->next_ = s->func_list_;
    s->func_list_ = def;
    return def;
}

static def_label_t* new_def_label(state_t* s)
{
    def_label_t* def = malloc(sizeof(def_label_t));
    assert(def);
    memset(def, 0, sizeof(def_label_t));
    def->next_ = s->label_list_;
    s->label_list_ = def;
    return def;
}

enum nj_param_type_t {
    nj_param_void,
    nj_param_int,
    nj_param_ptr,
    nj_param_label,
    nj_param_func,
};
typedef enum nj_param_type_t nj_param_type_t;

// instruction operand type lookup table
static const nj_param_type_t nj_param_type[] = {
        [nj_inst_unknown] = nj_param_void, [nj_inst_nop] = nj_param_void,
    [nj_inst_label] = nj_param_void, [nj_inst_debug] = nj_param_ptr,
    [nj_inst_const] = nj_param_int, [nj_inst_fp] = nj_param_void,
    [nj_inst_arg] = nj_param_int, [nj_inst_lget] = nj_param_int,
    [nj_inst_lset] = nj_param_int, [nj_inst_add] = nj_param_void,
    [nj_inst_sub] = nj_param_void, [nj_inst_mul] = nj_param_void,
    [nj_inst_mod] = nj_param_void, [nj_inst_div] = nj_param_void,
    [nj_inst_shl] = nj_param_void, [nj_inst_shr] = nj_param_void,
    [nj_inst_sra] = nj_param_void, [nj_inst_and] = nj_param_void,
    [nj_inst_xor] = nj_param_void, [nj_inst_or] = nj_param_void,
    [nj_inst_not] = nj_param_void, [nj_inst_lnot] = nj_param_void,
    [nj_inst_dup] = nj_param_void, [nj_inst_pop] = nj_param_int,
    [nj_inst_lt] = nj_param_void, [nj_inst_le] = nj_param_void,
    [nj_inst_eq] = nj_param_void, [nj_inst_ne] = nj_param_void,
    [nj_inst_ge] = nj_param_void, [nj_inst_gt] = nj_param_void,
    [nj_inst_ld] = nj_param_int, [nj_inst_st] = nj_param_int,
    [nj_inst_jmp] = nj_param_label, [nj_inst_cjmp] = nj_param_label,
    [nj_inst_call] = nj_param_func, [nj_inst_ret] = nj_param_int,
    [nj_inst_frame] = nj_param_int, [nj_inst_syscall] = nj_param_ptr,
};

// return instruction enum for token
static nj_inst_t match_inst(token_t* token)
{
    assert(token && token->start_);
    switch (token->start_[0]) {
    case ('a'):
        MATCH("arg", nj_inst_arg);
        MATCH("add", nj_inst_add);
        MATCH("and", nj_inst_and);
        break;
    case ('c'):
        MATCH("cjmp", nj_inst_cjmp);
        MATCH("call", nj_inst_call);
        MATCH("const", nj_inst_const);
        break;
    case ('d'):
        MATCH("div", nj_inst_div);
        MATCH("dup", nj_inst_dup);
        MATCH("debug", nj_inst_debug);
        break;
    case ('e'):
        MATCH("eq", nj_inst_eq);
        break;
    case ('f'):
        MATCH("fp", nj_inst_fp);
        MATCH("frame", nj_inst_frame);
        break;
    case ('g'):
        MATCH("ge", nj_inst_ge);
        MATCH("gt", nj_inst_gt);
        break;
    case ('j'):
        MATCH("jmp", nj_inst_jmp);
        break;
    case ('l'):
        MATCH("ld", nj_inst_ld);
        MATCH("lt", nj_inst_lt);
        MATCH("le", nj_inst_le);
        MATCH("lnot", nj_inst_lnot);
        MATCH("lget", nj_inst_lget);
        MATCH("lset", nj_inst_lset);
        MATCH("label", nj_inst_label);
        break;
    case ('m'):
        MATCH("mul", nj_inst_mul);
        MATCH("mod", nj_inst_mod);
        break;
    case ('n'):
        MATCH("ne", nj_inst_ne);
        MATCH("nop", nj_inst_nop);
        MATCH("not", nj_inst_not);
        break;
    case ('o'):
        MATCH("or", nj_inst_or);
        break;
    case ('p'):
        MATCH("pop", nj_inst_pop);
        break;
    case ('r'):
        MATCH("ret", nj_inst_ret);
        break;
    case ('s'):
        MATCH("st", nj_inst_st);
        MATCH("sub", nj_inst_sub);
        MATCH("shl", nj_inst_shl);
        MATCH("shr", nj_inst_shr);
        MATCH("sra", nj_inst_sra);
        MATCH("syscall", nj_inst_syscall);
        break;
    case ('x'):
        MATCH("xor", nj_inst_xor);
        break;
    }
    return nj_inst_unknown;
}

#define PAIR(ENUM, FUNC)                                                       \
    case (ENUM): {                                                             \
        FUNC(s->func_);                                                        \
    } break;

// parse instruction with no operands
static void parse_inst_void(nj_inst_t inst, state_t* s)
{
    assert(s);
    switch (inst) {
        PAIR(nj_inst_nop, nj_emit_nop);
        PAIR(nj_inst_fp, nj_emit_fp);
        PAIR(nj_inst_add, nj_emit_add);
        PAIR(nj_inst_sub, nj_emit_sub);
        PAIR(nj_inst_mul, nj_emit_mul);
        PAIR(nj_inst_mod, nj_emit_mod);
        PAIR(nj_inst_div, nj_emit_div);
        PAIR(nj_inst_shl, nj_emit_shl);
        PAIR(nj_inst_shr, nj_emit_shr);
        PAIR(nj_inst_sra, nj_emit_sra);
        PAIR(nj_inst_and, nj_emit_and);
        PAIR(nj_inst_xor, nj_emit_xor);
        PAIR(nj_inst_or, nj_emit_or);
        PAIR(nj_inst_not, nj_emit_not);
        PAIR(nj_inst_lnot, nj_emit_lnot);
        PAIR(nj_inst_dup, nj_emit_dup);
        PAIR(nj_inst_lt, nj_emit_lt);
        PAIR(nj_inst_le, nj_emit_le);
        PAIR(nj_inst_eq, nj_emit_eq);
        PAIR(nj_inst_ne, nj_emit_ne);
        PAIR(nj_inst_ge, nj_emit_ge);
        PAIR(nj_inst_gt, nj_emit_gt);
    default:
        assert(!"should not get here");
    }
}

#undef PAIR
#define PAIR(ENUM, FUNC)                                                       \
    case (ENUM): {                                                             \
        FUNC(s->func_, imm);                                                   \
    } break;

// parse instruction with single immediate argument
static void parse_inst_int(nj_inst_t inst, state_t* s)
{
    assert(s);
    token_t tok;
    next_token(s, &tok);
    int imm = 0;
    if (!token_as_int(&tok, &imm)) {
        assert(!"Expecting integer operand");
    }
    // parse an integer
    switch (inst) {
        PAIR(nj_inst_const, nj_emit_const);
        PAIR(nj_inst_arg, nj_emit_arg);
        PAIR(nj_inst_lget, nj_emit_lget);
        PAIR(nj_inst_lset, nj_emit_lset);
        PAIR(nj_inst_pop, nj_emit_pop);
        PAIR(nj_inst_ld, nj_emit_ld);
        PAIR(nj_inst_st, nj_emit_st);
        PAIR(nj_inst_ret, nj_emit_ret);
        PAIR(nj_inst_frame, nj_emit_ret);
    default:
        assert(!"Should not get here");
    }
}

#undef PAIR
#define PAIR(ENUM, FUNC)                                                       \
    case (ENUM): {                                                             \
        FUNC(s->func_, label);                                                 \
    } break;

// parse instruction with label operand
static void parse_inst_label(nj_inst_t inst, state_t* s)
{
    assert(s && s->func_);
    // read label name
    token_t tok;
    next_token(s, &tok);
    def_label_t* def = find_def_label(s, &tok);
    if (!def) {
        def = new_def_label(s);
        def->label_ = nj_label_create(s->func_);
        token_as_string(&tok, &def->name_);
    }
    // lookup or create label
    nj_label_t* label = def->label_;
    switch (inst) {
        PAIR(nj_inst_jmp, nj_emit_jmp);
        PAIR(nj_inst_cjmp, nj_emit_cjmp);
    default:
        assert(!"Should not get here");
    }
}

#undef PAIR

// parse instruction with function operand
static void parse_inst_func(nj_inst_t inst, state_t* s)
{
    assert(s);
    // parse function name
    token_t tok;
    next_token(s, &tok);
    // lookup a function by name
    def_func_t* def = find_def_func(s, &tok);
    assert(def && def->func_);
    // find or create func
    nj_func_t* func = def->func_;
    switch (inst) {
    case (nj_inst_call):
        nj_emit_call(s->func_, func);
        break;
    default:
        assert(!"Should not get here");
    }
}

// parse instruction with host pointer operand
static void parse_inst_ptr(nj_inst_t inst, state_t* s)
{
    assert(s);
    // parse syscall name
    token_t tok;
    next_token(s, &tok);
    // lookup syscall name
    nj_syscall_t syscall = NULL;
    switch (inst) {
    case (nj_inst_syscall):
        nj_emit_syscall(s->func_, syscall);
        break;
    default:
        assert(!"Should not get here");
    }
}

static void on_instruction(state_t* s, token_t* t)
{
    assert(s && t);
    nj_inst_t inst = match_inst(t);
    if (inst == nj_inst_unknown) {
        assert(!"unknown opcode");
    }
    switch (nj_param_type[inst]) {
    case (nj_param_void):
        parse_inst_void(inst, s);
        break;
    case (nj_param_int):
        parse_inst_int(inst, s);
        break;
    case (nj_param_label):
        parse_inst_label(inst, s);
        break;
    case (nj_param_func):
        parse_inst_func(inst, s);
        break;
    case (nj_param_ptr):
        parse_inst_ptr(inst, s);
        break;
    default:
        assert(!"should not get here");
    }
}

static void on_dot_func(state_t* s)
{
    // get token name
    token_t t;
    token_clear(&t);
    parse_next(&s->parser_, &t);
    // try to find find token name
    def_func_t* def = find_def_func(s, &t);
    if (!def) {
        def = new_def_func(s);
        token_as_string(&t, &def->name_);
        def->func_ = nj_func_create(s->cxt_, def->name_);
        assert(def->func_);
    }
    nj_func_place(def->func_);
    // make this function active
    s->func_ = def->func_;
    // munch any following tokens
    do {
        parse_next(&s->parser_, &t);
    } while (!token_is_eol(&t));
}

static void on_dot_proto(state_t* s)
{
    // get token name
    token_t t;
    token_clear(&t);
    parse_next(&s->parser_, &t);
    // try to find find token name
    def_func_t* def = find_def_func(s, &t);
    if (!def) {
        def = new_def_func(s);
        token_as_string(&t, &def->name_);
        def->func_ = nj_func_create(s->cxt_, def->name_);
        assert(def->func_);
    }
    do {
        parse_next(&s->parser_, &t);
    } while (!token_is_eol(&t));
}

static void on_dot_l(state_t* s, token_t* t)
{
    assert(s && t);
    assert(s->func_);
    def_label_t* def = find_def_label(s, t);
    if (!def) {
        def = new_def_label(s);
        token_as_string(t, &def->name_);
        def->label_ = nj_label_create(s->func_);
    }
    nj_label_place(s->func_, def->label_);
    token_t tok;
    token_clear(&tok);
    do {
        parse_next(&s->parser_, &tok);
    } while (!token_is_eol(&tok));
}

static void on_directive(state_t* s, token_t* t)
{
    assert(s && t);
    if (token_match(t, ".func")) {
        on_dot_func(s);
        return;
    }
    if (token_match(t, ".proto")) {
        on_dot_proto(s);
        return;
    }
    if (token_begins_with(t, ".l")) {
        on_dot_l(s, t);
        return;
    }
    // .test func [args] ret
    // .data name

    // munch any other tokens up to eol
    token_t tok;
    token_clear(&tok);
    do {
        parse_next(&s->parser_, &tok);
    } while (!token_is_eol(&tok));
}

#if 1
int main(int argc, char** args)
{
    atexit(getchar);

    buffer_t* buf = buff_create(0);
    assert(buf);
    if (!buff_load(buf, args[1])) {
        return 1;
    }

    state_t state;
    memset(&state, 0, sizeof(state));

    parse_init(&state.parser_, buff_data(buf, 0));
    token_t tok;
    token_clear(&tok);
    state.cxt_ = nj_context_create();
    assert(state.cxt_);

    while (true) {
        parse_next(&state.parser_, &tok);
        if (token_is_eol(&tok)) {
            continue;
        }
        if (token_is_eof(&tok)) {
            break;
        }
        if (*tok.start_ == '.') {
            on_directive(&state, &tok);
        } else {
            on_instruction(&state, &tok);
        }
    }

    nj_finish(state.cxt_);
    nj_disasm(state.cxt_);

    buff_free(buf);
    return 0;
}
#endif
