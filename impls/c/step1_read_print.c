#include <stdlib.h>
#include <stdio.h>
#include "types.h"
#include "reader.h"
#include "printer.h"
#include "linenoise/linenoise.h"

MalValue READ(char* str) {
    return read_str(str); 
}
MalValue EVAL(MalValue ast) {
    return ast;
}
char* PRINT(MalValue value) {
    return pr_str(value, true);
}
char* rep(char* str) {
    return PRINT(EVAL(READ(str)));
}

int main() {
    char* input;
    grow_stack();
    linenoiseHistorySetMaxLen(256);
    while(input = linenoise("user> ")) {
        char* str=input;
        while(is_space_or_comma(*str)) str++;
        if(*str) linenoiseHistoryAdd(str);
        printf("%s\n", rep(str));
        free(input);
        clear_stack();
    }
}
