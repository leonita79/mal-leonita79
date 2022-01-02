#include "reader.h"
#include <string.h>
#include <stdlib.h>

MalValue read_str(char* str) {
    Reader reader=(Reader){.next=str, .error={0}};
    reader_next(&reader);
    return read_form(&reader);
}

size_t reader_size(Reader* reader) {
    return reader->next-reader->token;
}
char* reader_peek(Reader* reader) {
    return reader->token;
}
char* reader_next(Reader* reader) {
    if(reader->error.type) return reader->token;
    while(reader->next[0] && is_space_or_comma(reader->next[0]))
        reader->next++;
    reader->token=reader->next;

    while(!is_space_or_comma(reader->next[0]) && !is_special(reader->next[0]))
        reader->next++;
    if(reader->next != reader->token)
        return reader->token; //string of nonspecial characters
    
    switch(reader->next[0]) {
        case '~':
            if(reader->next[1]=='@')
                reader->next++;
            break;
        case '"':
            reader->next++;
            while(reader->next[0] && reader->next[0] != '\"') {
                if(reader->next[0]=='\\' && reader->next[1])
                    reader->next++;
                reader->next++;
            }
            if(!reader->next[0])
                reader->error=make_errmsg("Unexpected end of input");
            break;
        case ';':     
            while(reader->next[0] && reader->next[0] != '\n') {
                reader->next++;
            }
            break;
    }
    if(reader->next[0])
        reader->next++; //first character of next token
    return reader->token;    
}
MalValue read_form(Reader* reader) {
    if(reader->error.type)
        return reader->error;
    if(!reader_peek(reader))
        return make_errmsg("Unexpected end of input");
    switch(reader_peek(reader)[0]) {
        case '(':
            return read_list(reader, MAL_TYPE_LIST, ')');
        case '[':
            return read_list(reader, MAL_TYPE_VECTOR, ']');
        case '{':
            return read_list(reader, MAL_TYPE_MAP, '}');
        case '\'':
            return read_quote(reader, "quote");
        case '`':
            return read_quote(reader, "quasiquote");
        case '~':
            return read_quote(reader, reader_peek(reader)[1]=='@' ? "splice-unquote" : "unquote");
        case '@':
            return read_quote(reader, "deref");
        case '^':
            return read_with_meta(reader);
        case ';':
            return make_const_atomic(MAL_TYPE_NIL, "nil", 3);
        default:
            return read_atom(reader);
    }
}
MalValue read_list(Reader* reader, uint8_t type, char delim) {
    uint32_t list_size=0;
    uint32_t list_capacity=4;
    MalValue* list=gc_alloc(list_capacity*sizeof(MalValue));
    if(reader->error.type)
        return reader->error;

    reader_next(reader);
    while(reader_peek(reader)[0] && reader_peek(reader)[0]!=delim) {
        if(list_capacity==list_size) {
            list_capacity*=2;
            list=gc_realloc(list, list_capacity*sizeof(MalValue));
        }
        list[list_size++]=read_form(reader);
        if(reader->error.type)
            return reader->error;
    }
    if(!reader_peek(reader)[0])
        return reader->error=make_errmsg("Unexpected end of input");
    reader_next(reader);
    return make_list(type, gc_realloc(list, list_size*sizeof(MalValue)), list_size);
}
MalValue read_atom(Reader* reader) {
    MalValue value;
    if(reader_peek(reader)[0]=='\"') {
        value=make_const_atomic(MAL_TYPE_STRING, reader_peek(reader), reader_size(reader));
    } else if(reader_peek(reader)[0]==':') {
        value=make_const_atomic(MAL_TYPE_KEYWORD, reader_peek(reader), reader_size(reader));
    } else if(strncmp("nil", reader_peek(reader), reader_size(reader))==0) {
        value=make_const_atomic(MAL_TYPE_NIL, reader_peek(reader), reader_size(reader));
    } else if(strncmp("false", reader_peek(reader), reader_size(reader))==0) {
        value=make_const_atomic(MAL_TYPE_FALSE, reader_peek(reader), reader_size(reader));
    } else if(strncmp("true", reader_peek(reader), reader_size(reader))==0) {
        value=make_const_atomic(MAL_TYPE_TRUE, reader_peek(reader), reader_size(reader));
    } else {
        char* tailptr=NULL;
        long number=strtol(reader_peek(reader), &tailptr, 10); 
        if(tailptr!=reader_peek(reader) && tailptr==reader_peek(reader)+reader_size(reader)) {
            value=make_number(number);
        } else {
            value=make_const_atomic(MAL_TYPE_SYMBOL, reader_peek(reader), reader_size(reader));
        }
    }
    if(reader->error.type)
        return reader->error;
    reader_next(reader);
    return value;
}

MalValue read_quote(Reader* reader, char* quote) {
    MalValue* list=gc_alloc(sizeof(MalValue)*2);
    reader_next(reader);    
    list[0]=make_const_atomic(MAL_TYPE_SYMBOL, quote, strlen(quote));
    list[1]=read_form(reader);
    if(reader->error.type)
        return reader->error;
    return make_list(MAL_TYPE_LIST, list, 2);
}
MalValue read_with_meta(Reader* reader) {
    MalValue* list=gc_alloc(sizeof(MalValue)*3);
    reader_next(reader);    
    list[0]=make_const_atomic(MAL_TYPE_SYMBOL, "with-meta", 9);
    list[2]=read_form(reader);
    list[1]=read_form(reader);
    if(reader->error.type)
        return reader->error;
    return make_list(MAL_TYPE_LIST, list, 3);
}

bool is_space_or_comma(char ch) {    
    switch(ch) {
        case ' ':
        case '\r':
        case '\n':
        case '\t':
        case ',':
            return true;
        default:
            return false;
    }
}
bool is_special(char ch) {    
    switch(ch) {
        case '\0':
        case '[':
        case ']':
        case '(':
        case ')':
        case '{':
        case '}':
        case '\'':
        case '`':
        case '^':
        case '@':
        case '~':
        case '\"':
        case ';':
            return true;
        default:
            return false;
    }
}

