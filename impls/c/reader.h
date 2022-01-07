#ifndef READER_H
#define READER_H

#include "types.h"

typedef struct {
    char* token;
    char* next;
    MalValue error;
} Reader;

MalValue read_str(char* str);

char* reader_peek(Reader* reader);
size_t reader_size(Reader* reader);
char* reader_next(Reader* reader);

MalValue read_form(Reader* reader);
MalValue read_list(Reader* reader, uint8_t type, char delim);
MalValue read_map(Reader* reader, uint8_t type, char delim);
MalValue read_atom(Reader* reader);

MalValue read_quote(Reader* reader, char* quote);
MalValue read_with_meta(Reader* reader);

bool is_space_or_comma(char ch);
bool is_special(char ch);

#endif //READER_H
