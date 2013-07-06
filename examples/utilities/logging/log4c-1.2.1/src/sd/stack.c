/* stack - a dynamically resizing stack
 * Copyright (c) 2001 Michael B. Allen <mballen@erols.com>
 *
 * The MIT License
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */ 

#include <sd/stack.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>

#include <sd/malloc.h>

#define SD_STACK_INIT_SIZE 32

struct __sd_stack {
    size_t max;
    size_t sp;
    size_t size;
    size_t iter;
    void **array;
};

/******************************************************************************/
sd_stack_t* sd_stack_new(size_t max)
{
    sd_stack_t* this;

    this        = sd_calloc(1, sizeof(sd_stack_t));
    this->max   = max == 0 ? INT_MAX : max;
    this->size  = SD_STACK_INIT_SIZE;
    this->sp    = 0;
    this->array = sd_calloc(this->size, sizeof(*this->array));

    return this;
}

/******************************************************************************/
void sd_stack_delete(sd_stack_t* this, void (*free_data_fn)(void *))
{
    if (!this)
	return;

    sd_stack_clear(this, free_data_fn);

    free(this->array);
    free(this);
}

/******************************************************************************/
size_t sd_stack_get_nelem(const sd_stack_t* this)
{
    return this ? this->sp : -1;
}

/******************************************************************************/
void sd_stack_clear(sd_stack_t* this, void (*free_data_fn)(void *))
{
    if (!this)
	return;

    if (free_data_fn) {
	while (this->sp > 0) {
	    free_data_fn(this->array[--(this->sp)]);
	}
    }
}

/******************************************************************************/
void* sd_stack_begin(sd_stack_t* this)
{
    if (!this)
	return NULL;

    this->iter = 0;
    return this->array[this->iter];
}

/******************************************************************************/
void* sd_stack_next(sd_stack_t* this)
{
    if (this && this->iter < this->sp) 
	return this->array[this->iter++];

    return NULL;
}

/******************************************************************************/
void* sd_stack_end(sd_stack_t* this)
{
    return sd_stack_peek(this);
}

/******************************************************************************/
void* sd_stack_peek(sd_stack_t* this)
{
    if (!this || !this->sp) 
	return NULL;

    return this->array[this->sp - 1];
}

/******************************************************************************/
int sd_stack_push(sd_stack_t* this, void *data)
{
    if (this == NULL)
	return -1;

    if (this->sp == this->size) {
	size_t new_size;

	if (this->size == this->max)
	    return -1;

	if (this->size * 2 > this->max) {
	    new_size = this->max;
	} else {
	    new_size = this->size * 2;
	}

	this->size = new_size;
	this->array = sd_realloc(this->array, sizeof(*this->array) * this->size);
    }

    assert(this->sp <= this->size);
    this->array[this->sp++] = data;
    return 0;
}

/******************************************************************************/
void* sd_stack_pop(sd_stack_t* this)
{
    if (this == NULL || this->sp == 0) 
	return NULL;

    if (this->size >= SD_STACK_INIT_SIZE * 4 && this->sp < this->size / 4) {
	size_t new_size = this->size / 2;

	this->size = new_size;
	this->array = sd_realloc(this->array, sizeof(*this->array) * this->size);
    }

    assert(this->sp > 0 && this->sp <= this->size);
    return this->array[--(this->sp)];
}

