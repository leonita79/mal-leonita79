#include "printer.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

char* pr_str(MalValue value, bool print_readably) {
    StringBuffer buffer={0};
    if(value.type==MAL_TYPE_ERRMSG && !value.as_str)
        return NULL;
    buffer.capacity=32;
    buffer.ptr=gc_alloc(buffer.capacity);

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
        case MAL_TYPE_NUMBER: 
            sb_printf(buffer, "%li", value.as_int);
            break;
        case MAL_TYPE_NIL:
        case MAL_TYPE_FALSE:
        case MAL_TYPE_TRUE:
        case MAL_TYPE_SYMBOL:
        case MAL_TYPE_STRING:
        case MAL_TYPE_KEYWORD:
        case MAL_TYPE_ERRMSG:
            sb_print_string(buffer, value.as_str, value.size);
            break;
        case MAL_TYPE_NATIVE_FUNCTION:
            sb_printf(buffer, "#<native %s>", value.as_native->name);
            break;
           default: 
           sb_print_string(buffer, "#<Error! Unknown type>>>", 22);
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
    if(buffer->size==buffer->capacity) {
        buffer->capacity*=2;
        buffer->ptr=gc_realloc(buffer->ptr, buffer->capacity);
    }
    buffer->ptr[buffer->size++]=ch;
}
void sb_print_string(StringBuffer* buffer, const char* string, size_t size) {
    if(buffer->size+size>buffer->capacity) {
        buffer->capacity*=2;
        buffer->ptr=gc_realloc(buffer->ptr, buffer->capacity);
    }
    memcpy(buffer->ptr+buffer->size, string, size);
    buffer->size+=size;
}
void sb_printf(StringBuffer* buffer, const char* fmt, ...) {
    va_list args1;
    va_list args2;
    int size;
    va_start(args1, fmt);
    va_copy(args2, args1);
    size=vsnprintf(NULL, 0, fmt, args1);
    if(buffer->size+size+1>buffer->capacity) {
        buffer->capacity*=2;
        buffer->ptr=gc_realloc(buffer->ptr, buffer->capacity);
    }
    vsnprintf(buffer->ptr+buffer->size, size+1, fmt, args2);
    va_end(args1);
    va_end(args2);
    buffer->size+=size;
}
