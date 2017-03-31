#include "parser.h"
#include <assert.h>

static int is_whitespace(char ch)
{
    switch (ch) {
    case ('\t'):
    case ('\r'):
    case (' '):
        return 1;
    default:
        return 0;
    }
}

static const char* parse_skip(const char* h)
{
    assert(h);
    while (is_whitespace(*h)) {
        ++h;
    }
    return h;
}

void parse_init(parser_t* p, const char* base)
{
    assert(p && base);
    p->base_ = base;
    p->head_ = base;
}

void parse_next(parser_t* p, token_t* out)
{
    assert(p && out);
    const char* h = parse_skip(p->head_);
    // skip over comments
    if (*h == ';') {
        while (*h && *h != '\n') {
            ++h;
        }
    }
    out->start_ = h;
    // if its a control char
    if (*h < ' ') {
        out->end_ = h + 1;
        p->head_ = (*h == '\0') ? h : h + 1;
    } else {
        // advance till '\0' or whitespace
        for (; *h && !is_whitespace(*h);) {
            ++h;
        }
        p->head_ = h;
        out->end_ = h;
    }
}
