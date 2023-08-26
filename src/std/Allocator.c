#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "Result.h"
#include "Str.h"
#include "defs.h"
#include "panic.c"
#include <stddef.h>

typedef void *Realloc(void *ptr, size_t size, size_t old_size, void *state);

typedef struct {
  Realloc *realloc;
  void *state;
} Allocator;

static Result(Str) alloc(Allocator ally, size_t size) {
  void *ptr = ally.realloc(NULL, size, 0, ally.state);
  if (!ptr) {
    return Result_Err(Str, "alloc: out of memory");
  }
  Str res = {
      .ptr = ally.realloc(NULL, size, 0, ally.state),
      .len = size,
  };
  return Result_Ok(Str, res);
}

static void resizeAllocation(Allocator ally, Str *allocation, size_t new_size) {
  void *ptr =
      ally.realloc(allocation->ptr, new_size, allocation->len, ally.state);
  if (!ptr) {
    return;
  }
  allocation->ptr = ptr;
  allocation->len = new_size;
}

typedef struct {
  Str mem;
  size_t cur;
} Bump;

static void *bumpRealloc(void *ptr, size_t size, size_t old_size, void *state) {
  Bump *bump = (Bump *)state;
  if (size == 0) {
    return NULL;
  }
  if (ptr != NULL) {
    if (bump->mem.ptr + bump->cur - old_size != ptr) {
      return NULL;
    }
  }

  size_t align = (8 - (((size_t)bump->mem.ptr + bump->cur) % 8)) % 8;

  if (bump->cur - old_size + size + align > bump->mem.len) {
    return NULL;
  }
  void *result = bump->mem.ptr + bump->cur - old_size + align;
  bump->cur += size - old_size;
  return result;
}

#endif /* ALLOCATOR_H */