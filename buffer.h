#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

// the buffer object
typedef struct buffer_t buffer_t;

// create a buffer object
buffer_t* buff_create(size_t size);

// release a buffer object
void buff_free(buffer_t * buf);

// access the base buffer data
void * buff_data(buffer_t * buf, uintptr_t offset);

// return the availible size of the buffer
size_t buff_size(buffer_t * buff);

// return the current read/write head
uintptr_t buff_head(buffer_t *buf);

// resize buffer to a specific size
void buff_resize(buffer_t * buf, size_t size);

// seek the read/write head to a specific position
void buff_seek(buffer_t * buf, uintptr_t pos);

// write data to the buffer
void buff_write(buffer_t * buf, const void * src, size_t size);

// read data from the buffer
void buff_read(buffer_t * buf, void * dst, size_t size);

#endif // _BUFFER_H_
