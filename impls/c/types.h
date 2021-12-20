#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

enum {
    MAL_TYPE_LIST,
    MAL_TYPE_SYMBOL
};

typedef struct MalValue {
    uint8_t type;
    uint32_t size;
    union {
        char* as_str;
        struct MalValue* as_list;
    };
} MalValue;

void clear_stack();
void grow_stack();
void* stack_alloc(size_t size);
void* stack_realloc(void* ptr, size_t size);

MalValue mal_copy(MalValue value);
void mal_free(MalValue value);

MalValue make_list(MalValue* data, uint32_t size);
MalValue make_symbol(char* name, uint32_t size);
#endif //TYPES_H

