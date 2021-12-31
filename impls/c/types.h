#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

enum {
    MAL_TYPE_NIL,
    MAL_TYPE_FALSE,
    MAL_TYPE_TRUE,
    MAL_TYPE_LIST,
    MAL_TYPE_VECTOR,
    MAL_TYPE_MAP,
    MAL_TYPE_SYMBOL,
    MAL_TYPE_STRING,
    MAL_TYPE_KEYWORD,
    MAL_TYPE_ERRMSG
};

typedef struct MalValue {
    uint8_t type;
    uint8_t is_gc;
    uint32_t size;
    union {
        char* as_str;
        struct MalValue* as_list;
    };
} MalValue;

void* gc_alloc(size_t size);
void* gc_realloc(void* ptr, size_t size);
void gc_mark(MalValue value);
void gc_collect(bool full);
void gc_destroy();

MalValue mal_copy(MalValue value);

MalValue make_list(uint8_t type, MalValue* data, uint32_t size);
MalValue make_const_atomic(uint8_t type, char* string, uint32_t size);
MalValue make_errmsg(char* msg);
#endif //TYPES_H

