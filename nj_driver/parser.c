#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

static bool is_whitespace(char ch)
{
    return ch == '\t' || ch == '\r' || ch == ' ';
}

// skip all whitespace
static const char* parse_skip(const char* h)
{
    assert(h);
    while (is_whitespace(*h)) {
        ++h;
    }
    return h;
}

static bool char_dec(char c, int* out)
{
    if (c >= '0' && c <= '9') {
        *out = c - '0';
        return true;
    }
    return false;
}

static bool char_hex(char c, int* out)
{
    if (c >= '0' && c <= '9') {
        *out = (c - '0');
        return true;
    }
    if (c >= 'a' && c <= 'a') {
        *out = 10 + (c - 'a');
        return true;
    }
    if (c >= 'A' && c <= 'A') {
        *out = 10 + (c - 'A');
        return true;
    }
    return false;
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
    // keep track of the line count
    (*h == '\n') ? ++p->line_ : 0;
}

bool token_is_eol(token_t* tok)
{
    assert(tok);
    if (tok->start_) {
        return *(tok->start_) == '\n';
    }
    return false;
}

bool token_is_eof(token_t* tok)
{
    assert(tok);
    if (tok->start_) {
        return *(tok->start_) == '\0';
    }
    return false;
}

void token_clear(token_t* tok)
{
    assert(tok);
    memset(tok, 0, sizeof(token_t));
}

size_t token_length(token_t* tok)
{
    assert(tok);
    assert(tok->end_ >= tok->start_);
    const size_t size = (uintptr_t)tok->end_ - (uintptr_t)tok->start_;
    return size;
}

bool token_as_string(token_t* tok, char** out)
{
    assert(tok && out);
    const size_t len = token_length(tok);
    if (len <= 0) {
        return false;
    }
    char* temp = (char*)malloc(len + 1);
    memcpy(temp, tok->start_, len);
    temp[len] = '\0';
    *out = temp;
    return true;
}

bool token_as_int(token_t* tok, int* out)
{
    assert(tok && out);
    const size_t len = token_length(tok);
    if (len <= 0) {
        return false;
    }
    int accum = 0;
    const char* src = tok->start_;
    int base = 10;
    bool (*convert)(char c, int* out) = char_dec;
    if (*src == 'h') {
        base = 16;
        convert = char_hex;
        ++src;
    }
    for (; src != tok->end_; ++src) {
        int digit = 0;
        if (!convert(*src, &digit)) {
            return false;
        }
        accum *= base;
        accum += digit;
    }
    *out = accum;
    return true;
}

bool token_match(token_t* tok, const char* rhs)
{
    assert(tok && rhs);
    const size_t len_a = token_length(tok);
    const size_t len_b = strlen(rhs);
    if (len_a != len_b) {
        return false;
    }
    return memcmp(tok->start_, rhs, len_a) == 0;
}

bool token_begins_with(token_t* tok, const char* rhs)
{
    assert(tok && rhs);
    const size_t len_a = token_length(tok);
    const size_t len_b = strlen(rhs);
    if (len_a < len_b) {
        return false;
    }
    return memcmp(tok->start_, rhs, len_b) == 0;
}
