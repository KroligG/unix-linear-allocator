#define _GNU_SOURCE

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

void* _current_pos = NULL;
void* _start_pos = NULL;
void* _max_pos = NULL;
size_t _used_memory = 0;
size_t _num_allocations = 0;

static void* (*real_malloc)(size_t)=NULL;

inline size_t get_align_adjustment(const void* address, size_t alignment)
{
    size_t adjustment =  alignment - ( (uintptr_t)(address) & (uintptr_t)(alignment-1) );
    if(adjustment == alignment)
        return 0;

    return adjustment;
}

void* malloc(size_t size)
{
    size_t TOTAL_SIZE = 1e9;
    if(_current_pos==NULL) {
        real_malloc = dlsym(RTLD_NEXT, "malloc");
        _start_pos = real_malloc(TOTAL_SIZE);
        _current_pos = _start_pos;
        _max_pos = (char*)_start_pos + TOTAL_SIZE;
        _used_memory = 0;
        _num_allocations = 0;
    }

    size_t adjustment =  get_align_adjustment(_current_pos, 4);
  	if(_current_pos + adjustment + size > _max_pos) {
		return NULL;
    }
	uintptr_t aligned_address = (uintptr_t)_current_pos + adjustment;
	_current_pos = (void*)(aligned_address + size);
	_used_memory += size + adjustment;
	_num_allocations++;
	fprintf(stdout, "malloc(%d) = %p. used memory: %d, allocations: %d\n",
        size, (void*)aligned_address, _used_memory, _num_allocations);
	return (void*)aligned_address;
}

void* realloc(void *ptr, size_t size) {
    fprintf(stdout, "realloc(%p, %d)\n", ptr, size);
    return NULL;
}

void free(void* ptr) {
    fprintf(stdout, "free(%p)\n", ptr);
}
