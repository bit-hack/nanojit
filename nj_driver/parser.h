#pragma once

#include <stdbool.h>
#include <stdint.h>

struct parser_t {
    const char* base_;
    const char* head_;
    uint32_t line_;
};
typedef struct parser_t parser_t;

struct token_t {
    const char* start_;
    const char* end_;
};
typedef struct token_t token_t;

bool token_is_eol(token_t* tok);
bool token_is_eof(token_t* tok);

// clear a token structure
void token_clear(token_t* tok);

// return the string length of a token
size_t token_length(token_t* tok);

// extract a string from this token which can later be released using free()
bool token_as_string(token_t*, char** out);

// try to interpret this token as an integer
bool token_as_int(token_t*, int* out);

// try to string match
bool token_match(token_t*, const char* rhs);

// match the start of a string
bool token_begins_with(token_t*, const char* str);

// initalize a parser object
void parse_init(parser_t*, const char* base);

// extract the next token
void parse_next(parser_t*, token_t* out);
