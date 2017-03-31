#pragma once

struct parser_t {
    const char * base_;
    const char * head_;
};
typedef struct parser_t parser_t;

struct token_t {
    const char * start_;
    const char * end_;
};
typedef struct token_t token_t;

void parse_init(parser_t *, const char * base);

void parse_next(parser_t *, token_t * out);
