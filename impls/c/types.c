#include "types.h"
#include <stdlib.h>
#include <string.h>

void clear_stack() {
}
void grow_stack() {
}
void* stack_alloc(size_t size) {
    return malloc(size);
}
void* stack_realloc(void* ptr, size_t size) {
    return realloc(ptr, size);
}

MalValue mal_copy(MalValue value) {
    return value;
}
void mal_free(MalValue value) {
    return;
}

MalValue make_list(MalValue* list, uint32_t size) {
    MalValue value=(MalValue){.type=MAL_TYPE_LIST, .size=size};
    value.as_list=list;
    return value;
}
MalValue make_symbol(char* name, uint32_t size) {
    MalValue value=(MalValue){.type=MAL_TYPE_SYMBOL, .size=size};
    value.as_str=name;
    return value;
}

