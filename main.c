#define _GNU_SOURCE

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>

static void *_current_pos = NULL;
static void *_start_pos = NULL;
static void *_max_pos = NULL;
static size_t _used_memory = 0;
static size_t _num_allocations = 0;

static void *(*real_malloc)(size_t) = NULL;

void *malloc(size_t size) {
    size_t TOTAL_SIZE = 1024 * 1024 * 1024;
    if (_current_pos == NULL) {
        real_malloc = dlsym(RTLD_NEXT, "malloc");
        _start_pos = real_malloc(TOTAL_SIZE);
        _current_pos = _start_pos;
        _max_pos = (char *) _start_pos + TOTAL_SIZE;
        _used_memory = 0;
        _num_allocations = 0;
    }

    if (_current_pos + size > _max_pos) {
        return NULL;
    }
    uintptr_t aligned_address = (uintptr_t) _current_pos + sizeof(size_t);
    *(size_t *) _current_pos = size;
    _current_pos = (void *) (aligned_address + size);
    _num_allocations++;
    _used_memory += size;
//    fprintf(stdout, "malloc(%ld) = %p. used memory: %ld, allocations: %ld\n",
//            size, (void *) aligned_address, _used_memory, _num_allocations);
    return (void *) aligned_address;
}

void *realloc(void *ptr, size_t size) {
//    fprintf(stdout, "realloc(%p, %ld)\n", ptr, size);
    void *mem = malloc(size);
    if (!mem) {
        return NULL;
    }
    if (ptr != NULL) {
        size_t *old_size = (size_t *) ptr - 1;
        memcpy(ptr, mem, *old_size);
    }
    return mem;
}

void free(void *ptr) {
//    fprintf(stdout, "free(%p)\n", ptr);
}
