#include "printer.h"
#include <string.h>

char* pr_str(MalValue value, bool print_readably) {
    StringBuffer buffer={0};
    if(value.type==MAL_TYPE_ERRMSG && !value.as_str)
        return NULL;
    buffer.capacity=32;
    buffer.ptr=stack_alloc(buffer.capacity);

    print_value(&buffer, value, print_readably);
    sb_print_char(&buffer, '\0');
    return buffer.ptr;
}
void print_value(StringBuffer* buffer, MalValue value, bool print_readably) {
    switch(value.type) {
        case MAL_TYPE_LIST: 
            print_list(buffer, '(', value, ')', print_readably);
            break;
        case MAL_TYPE_VECTOR:
            print_list(buffer, '[', value, ']', print_readably);
            break;
        case MAL_TYPE_MAP:
            print_list(buffer, '{', value, '}', print_readably);
            break;
        case MAL_TYPE_SYMBOL:
        case MAL_TYPE_ERRMSG:
            sb_print_string(buffer, value.as_str, value.size);
            break;
    }
}

void print_list(StringBuffer* buffer, char open, MalValue value, char close, bool print_readably) {
    sb_print_char(buffer, open);
    for(int i=0; i<value.size; i++) {
        if(i>0)
            sb_print_char(buffer, ' ');
        print_value(buffer, value.as_list[i], print_readably);    
    }
    sb_print_char(buffer, close);
}

void sb_print_char(StringBuffer* buffer, char ch) {
    if(!buffer->ptr) return;
    if(buffer->size==buffer->capacity) {
        buffer->capacity*=2;
        buffer->ptr=stack_realloc(buffer->ptr, buffer->capacity);
        if(!buffer->ptr) return;
    }
    buffer->ptr[buffer->size++]=ch;
}
void sb_print_string(StringBuffer* buffer, char* string, size_t size) {
    if(!buffer->ptr) return;
    if(buffer->size+size>buffer->capacity) {
        buffer->capacity*=2;
        buffer->ptr=stack_realloc(buffer->ptr, buffer->capacity);
        if(!buffer->ptr) return;
    }
    memcpy(buffer->ptr+buffer->size, string, size);
    buffer->size+=size;
}
