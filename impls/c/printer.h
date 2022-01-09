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
void print_list(StringBuffer* buffer, char open, MalValue value, char close, bool print_readably);
void print_map(StringBuffer* buffer, char open, MalValue value, char close, bool print_readably);
void print_string(StringBuffer* buffer, MalValue value, bool print_readably);
void sb_print_char(StringBuffer* buffer, char ch);
void sb_print_string(StringBuffer* buffer, const char* string, size_t size);
void sb_printf(StringBuffer* buffer, const char* fmt, ...);

#endif //PRINTER_H

