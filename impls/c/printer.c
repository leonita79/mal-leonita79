#include "printer.h"
#include <string.h>
#include <stdio.h>

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
        case MAL_TYPE_NUMBER: {
            int size=snprintf(NULL, 0, "%li", value.as_int);
            char* temp=sb_print_string(buffer, NULL, size);
            snprintf(temp, size+1, "%li", value.as_int);
            break;
        }
        case MAL_TYPE_NIL:
        case MAL_TYPE_FALSE:
        case MAL_TYPE_TRUE:
        case MAL_TYPE_SYMBOL:
        case MAL_TYPE_STRING:
        case MAL_TYPE_KEYWORD:
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
    if(buffer->size==buffer->capacity) {
        buffer->capacity*=2;
        buffer->ptr=gc_realloc(buffer->ptr, buffer->capacity);
    }
    buffer->ptr[buffer->size++]=ch;
}
char* sb_print_string(StringBuffer* buffer, char* string, size_t size) {
    if(buffer->size+size+1>buffer->capacity) {
        buffer->capacity*=2;
        buffer->ptr=gc_realloc(buffer->ptr, buffer->capacity);
    }
    if(string)
        memcpy(buffer->ptr+buffer->size, string, size);
    buffer->size+=size;
    return buffer->ptr+buffer->size-size;
}
