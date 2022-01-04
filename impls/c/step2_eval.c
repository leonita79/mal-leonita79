#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "reader.h"
#include "printer.h"
#include "linenoise/linenoise.h"

MalValue native_add(uint32_t size, MalValue* args) {
    long value=0;
    for(int i=0; i<size; i++)
        value+=args[i].as_int;
    return make_number(value);
}
MalValue native_sub(uint32_t size, MalValue* args) {
    long value=size>0 ? args[0].as_int : 0;
    for(int i=1; i<size; i++)
        value-=args[i].as_int;
    return make_number(value);
}
MalValue native_mul(uint32_t size, MalValue* args) {
    long value=1;
    for(int i=0; i<size; i++)
        value*=args[i].as_int;
    return make_number(value);
}
MalValue native_div(uint32_t size, MalValue* args) {
    long value=size>0 ? args[0].as_int : 0;
    for(int i=1; i<size; i++)
        value/=args[i].as_int;
    return make_number(value);
}

MalNativeData repl_env[]={
    {"+", &native_add},
    {"-", &native_sub},
    {"*", &native_mul},
    {"/", &native_div}
};

MalValue READ(char* str) {
    return read_str(str); 
}
MalValue eval_symbol(MalValue ast, MalNativeData* env) {
    for(int i=0; i<4; i++) {
        if(strncmp(ast.as_str,repl_env[i].name, ast.size)==0) {
            MalValue value={.type=MAL_TYPE_NATIVE_FUNCTION, .is_gc=0};
            value.as_native=&repl_env[i];
            return value;
        }
    }
    return make_errmsg("Symbol not defined");
}
MalValue EVAL(MalValue ast, MalNativeData* env);
MalValue eval_list(MalValue ast, MalNativeData* env) {
    if(ast.size==0) return ast;
    MalValue* list=gc_alloc(ast.size*sizeof(MalValue));
    for(int i=0; i<ast.size; i++) {
        list[i]=EVAL(ast.as_list[i], env);
        switch(list[i].type) {
            case MAL_TYPE_ERRMSG:
                return list[i];
        }
    }
    return make_list(ast.type, list, ast.size);
}
MalValue eval_ast(MalValue ast, MalNativeData* env) {
    switch(ast.type) {
        case MAL_TYPE_SYMBOL:
            return eval_symbol(ast, env);
        case MAL_TYPE_LIST:
        case MAL_TYPE_VECTOR:
        case MAL_TYPE_MAP:
            return eval_list(ast, env);
        default:
            return ast;
    }
}
MalValue EVAL(MalValue ast, MalNativeData* env) {
    MalValue value=eval_ast(ast, env);
    if(ast.type==MAL_TYPE_LIST && ast.size>0) {
        MalValue fn=value.as_list[0];
        switch(fn.type) {
            case MAL_TYPE_NATIVE_FUNCTION:
                return fn.as_native->fn(value.size-1, value.as_list+1);
            default:
                return make_errmsg("Can't call non-function");
        }
    }
    return value;
}
char* PRINT(MalValue value) {
    return pr_str(value, true);
}
char* rep(char* str) {
    return PRINT(EVAL(READ(str), repl_env));
}

int main() {
    char* input;
    linenoiseHistorySetMaxLen(256);
    while(input = linenoise("user> ")) {
        char* str=input;
        while(is_space_or_comma(*str)) str++;
        if(*str) {
            linenoiseHistoryAdd(str);
            printf("%s\n", rep(str));
        }
        free(input);
        gc_collect(false);
    }
    gc_destroy();
}
