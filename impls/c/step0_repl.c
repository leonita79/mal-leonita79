#include <stdio.h>
#include <malloc.h>
#include "linenoise/linenoise.h"

char* READ(char* str) { return str; }
char* EVAL(char* str) { return str; }
char* PRINT(char* str) { return str; }
char* rep(char* str) {
    return PRINT(EVAL(READ(str)));
}

int main() {
    char* str;
    linenoiseHistorySetMaxLen(256);
    while((str = linenoise("user> ")) != NULL) {
        printf("%s\n", rep(str));
        if(str[0] && str[0]!='\n')
            linenoiseHistoryAdd(str);
        free(str);
    }
}
