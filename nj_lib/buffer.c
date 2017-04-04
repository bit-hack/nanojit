#define _CRT_SECURE_NO_WARNINGS
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"

#define MAX2(A, B) (((A) > (B)) ? (A) : (B))
#define MIN_SIZE 512u

typedef struct buffer_t {
    uint8_t* data_;
    uintptr_t head_;
    size_t size_;
    size_t alloc_size_;
} buffer_t;

typedef struct buffer_t buffer_t;

static uint64_t NPOT(uint64_t in)
{
    in -= (in != 0);
    in |= in >> 1u;
    in |= in >> 2u;
    in |= in >> 4u;
    in |= in >> 8u;
    in |= in >> 16u;
    in |= in >> 32u;
    return ++in;
}

// return the size of a file
static size_t fsize(FILE* fd)
{
    const long p = ftell(fd);
    fseek(fd, 0, SEEK_END);
    const size_t size = ftell(fd);
    fseek(fd, p, SEEK_SET);
    return size;
}

// make sure the buffer has at least 'size' bytes in it
static void buff_reserve(buffer_t* buf, size_t size)
{
    assert(buf);
    const size_t new_size = MAX2((size_t)NPOT(size), MIN_SIZE);

    if (buf->data_ == NULL) {
        buf->data_ = malloc(new_size);
        buf->alloc_size_ = new_size;
        assert(buf->data_);
        return;
    }

    // TODO: if new_size is significantly less then old_size we can shrink

    if (new_size != buf->alloc_size_) {
        buf->data_ = realloc(buf->data_, new_size);
        assert(buf->data_);
        buf->alloc_size_ = new_size;
        return;
    }
}

buffer_t* buff_create(size_t size)
{
    buffer_t* buf = malloc(sizeof(buffer_t));
    assert(buf);
    memset(buf, 0, sizeof(buffer_t));
    buff_reserve(buf, 0);
    return buf;
}

void buff_free(buffer_t* buf)
{
    assert(buf);
    if (buf->data_) {
        free(buf->data_);
    }
    free(buf);
}

void* buff_data(buffer_t* buf, uintptr_t offset)
{
    assert(buf);
    assert(offset < buf->size_);
    return buf->data_ + offset;
}

size_t buff_size(buffer_t* buff)
{
    assert(buff);
    if (buff->data_ == NULL) {
        assert(buff->size_ == 0);
    }
    return buff->size_;
}

void buff_resize(buffer_t* buf, size_t size)
{
    assert(buf);
    buff_reserve(buf, size);
    buf->size_ = size;
}

void buff_seek(buffer_t* buf, uintptr_t pos)
{
    assert(buf);
    buf->head_ = pos;
}

void buff_write(buffer_t* buf, const void* src, size_t size)
{
    assert(buf && src);
    // resize if needed
    const uintptr_t old_size = buf->size_;
    const uintptr_t new_size = buf->head_ + size;
    buff_reserve(buf, buf->head_ + size);
    buf->size_ = MAX2(old_size, new_size);
    // do the actual write
    uint8_t* dst = buf->data_ + buf->head_;
    memcpy(dst, src, size);
    buf->head_ += size;
}

void buff_read(buffer_t* buf, void* dst, size_t size)
{
    assert(buf);
    uintptr_t end = buf->head_ + size;
    if (end <= buf->size_) {
        memcpy(dst, buf->data_ + buf->head_, size);
        buf->head_ += size;
    } else {
        assert(!"Atempt to read past end of buffer");
    }
}

uintptr_t buff_head(buffer_t* buf)
{
    assert(buf);
    return buf->head_;
}

bool buff_load(buffer_t* b, const char* path)
{
    assert(path && b);
    FILE* fd = fopen(path, "rb");
    if (fd == NULL) {
        return false;
    }
    const size_t size = fsize(fd);
    buff_resize(b, size + 1);
    char* dst = buff_data(b, 0);
    if (fread(dst, 1, size, fd) != size) {
        fclose(fd);
        return false;
    }
    fclose(fd);
    dst[size] = '\0';
    return true;
}

bool buff_save(buffer_t* b, const char* path)
{
    assert(b && path);
    FILE* fd = fopen(path, "wb");
    if (fd == NULL) {
        return false;
    }
    const void* data = buff_data(b, 0);
    assert(data);
    const size_t size = buff_size(b);
    size_t ret = fwrite(data, 1, size, fd);
    fclose(fd);
    return ret == size;
}
