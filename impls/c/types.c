#include "types.h"
#include <stdlib.h>
#include <string.h>

char* stack=NULL;
size_t stack_last=NULL;
size_t stack_size=0;
size_t stack_capacity=1024;

void clear_stack() {
    stack_size=0;
}
void grow_stack() {
    if(stack) stack_capacity*=2;
    stack=realloc(stack, stack_capacity);
}
void free_stack() {
    free(stack);
}
void* stack_alloc(size_t size) {
    stack_last=stack_size;
    stack_size+=size+(7-(size+7)%8); //round to 8
    if(stack_size>stack_capacity)
        return NULL;
    return stack+stack_last;
}
void* stack_realloc(void* ptr, size_t size) {
    if(ptr==stack+stack_last) {
        stack_size=stack_last+size+(7-(size+7)%8); //round to 8
        if(stack_size>stack_capacity)
            return NULL;
        return ptr;    
    } else {
        void* new_ptr=stack_alloc(size);
        memcpy(new_ptr, ptr, size);
        return new_ptr;
    }
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

