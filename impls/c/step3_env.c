#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "reader.h"
#include "printer.h"
#include "env.h"
#include "linenoise/linenoise.h"

MalValue EVAL(MalValue ast, MalEnv* env);
MalValue fn_add(uint32_t size, MalValue* args) {
    long value=0;
    for(int i=0; i<size; i++)
        value+=args[i].as_int;
    return make_number(value);
}
MalValue fn_sub(uint32_t size, MalValue* args) {
    long value=size>0 ? args[0].as_int : 0;
    for(int i=1; i<size; i++)
        value-=args[i].as_int;
    return make_number(value);
}
MalValue fn_mul(uint32_t size, MalValue* args) {
    long value=1;
        for(int i=0; i<size; i++)
        value*=args[i].as_int;
    return make_number(value);
}
MalValue fn_div(uint32_t size, MalValue* args) {
    long value=size>0 ? args[0].as_int : 0;
    for(int i=1; i<size; i++)
        value/=args[i].as_int;
    return make_number(value);
}

bool special_def_bang(MalValue* ast, MalEnv* env) {
    if(ast->size<2 || ast->as_list[1].type != MAL_TYPE_SYMBOL) {
        *ast=make_errmsg("Bad def!");
        return false;
    }
    MalValue value=EVAL(ast->as_list[2], env);
    switch(value.type) {
        case MAL_TYPE_ERRMSG:
            *ast=value;
            return false;
    }
    *env=env_set(*env, ast->as_list[1], value);
    *ast=value;
    return false;
}
bool special_let_star(MalValue* ast, MalEnv* env) {
    if(ast->size<2) {
        *ast=make_errmsg("Bad let*");
        return false;
    }
    MalValue bindings=ast->as_list[1];
    switch(bindings.type) {
        case MAL_TYPE_LIST:
        case MAL_TYPE_VECTOR:
            if(bindings.size%2==0) break;
            //else fall through
        default:
            *ast=make_errmsg("Bad let*");
            return false;
    }
    MalEnv new_env=env_init(*env, bindings.size/2);
    for(int i=0; i<bindings.size; i+=2) {
        if(bindings.as_list[i].type!=MAL_TYPE_SYMBOL) {
            *ast=make_errmsg("Bad let*");
            return false;
        }
        MalValue value=EVAL(bindings.as_list[i+1], &new_env);
        switch(value.type) {
            case MAL_TYPE_ERRMSG:
                *ast=value;
                return false;
        }
        new_env=env_set(new_env, bindings.as_list[i], value);
    }
    *ast=EVAL(ast->as_list[2], &new_env);
    return false;
}

MalNativeData repl_env_data[]={
    {"+", &fn_add},
    {"-", &fn_sub},
    {"*", &fn_mul},
    {"/", &fn_div}
};
MalSpecialData special_form_data[]={
    {"def!", &special_def_bang},
    {"let*", &special_let_star},
};
MalEnv repl_env;
MalEnv special_forms;

MalValue READ(char* str) {
    return read_str(str); 
}
MalValue eval_list(MalValue ast, MalEnv* env) {
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
MalValue eval_map(MalValue ast, MalEnv* env) {
    if(ast.as_list[0].size==0) return ast;
    MalValue* map=map_init(ast.as_list[0].size);
    for(size_t i=1; i<=ast.size; i++) {
        if(ast.as_list[i].type) {
            MalValue value=EVAL(ast.as_list[i+ast.size], env);
            switch(value.type) {
                case MAL_TYPE_ERRMSG:
                    return value;
            }
            map=map_set(map, ast.as_list[i], value);
        }
    }
    return make_map(map);
}
MalValue eval_ast(MalValue ast, MalEnv* env) {
    switch(ast.type) {
        case MAL_TYPE_SYMBOL:
            return env_get(*env, ast);
        case MAL_TYPE_LIST:
        case MAL_TYPE_VECTOR:
            return eval_list(ast, env);
        case MAL_TYPE_MAP:
            return eval_map(ast, env);
        default:
            return ast;
    }
}
MalValue EVAL(MalValue ast, MalEnv* env) {
    if(ast.type==MAL_TYPE_LIST && ast.size>0) {
        MalValue* special=(ast.as_list[0].type==MAL_TYPE_SYMBOL)
            ? map_get(special_forms, ast.as_list[0])
            : NULL;

        if(special) {
            special->as_special->fn(&ast, env);
            return ast;
        }
            
        MalValue value=eval_ast(ast, env);
        switch(value.type) {
            case MAL_TYPE_ERRMSG:
                return value;
        }
        MalValue fn=value.as_list[0];
        switch(fn.type) {
            case MAL_TYPE_NATIVE_FUNCTION:
                return fn.as_native->fn(value.size-1, value.as_list+1);
            default:
                return make_errmsg("Can't call non-function");
        }
    }
    return eval_ast(ast, env);
}
char* PRINT(MalValue value) {
    return pr_str(value, true);
}
char* rep(char* str) {
    return PRINT(EVAL(READ(str), &repl_env));
}

int main() {
    char* input;
    int size=sizeof(repl_env_data)/sizeof(repl_env_data[0]);
    repl_env=env_init(NULL, 10);
    for(int i=0; i<size; i++) {
        repl_env=env_set_native(repl_env, &repl_env_data[i]);
    }
    size=sizeof(special_form_data)/sizeof(special_form_data[0]);
    special_forms=env_init(NULL, size);
    for(int i=0; i<size; i++) {
        special_forms=env_set_special(special_forms, &special_form_data[i]);
    }
    linenoiseHistorySetMaxLen(256);
    while(input = linenoise("user> ")) {
        char* str=input;
        while(is_space_or_comma(*str)) str++;
        if(*str) {
            linenoiseHistoryAdd(str);
            printf("%s\n", rep(str));
        }
        gc_mark_env(repl_env);
        gc_mark_env(special_forms);
        free(input);
        gc_collect();
    }
    gc_destroy();
}
