#ifndef PRINTER_H
#define PRINTER_H

#include "types.h"

typedef struct {
    char* ptr;
    size_t size;
    size_t capacity;
} StringBuffer;

char* pr_str(MalValue value, bool print_readably);
void print_value(StringBuffer* buffer, MalValue value, bool print_readably);
void sb_print_char(StringBuffer* buffer, char ch);
void sb_print_string(StringBuffer* buffer, char* string, size_t size);

#endif //PRINTER_H

