#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

void genFun(Fun * p) {
    printf("    .global %s\n", p->name);
    printf("%s:\n", p->name);
    printf("    mov $0,%%rax\n");
    printf("    ret\n");
}

void genFuns(Funs * p) {
    if (p == 0)
        return;
    genFun(p->first);
    genFuns(p->rest);
}

int main(int argc, char *argv[]) {
    Funs *p = parse();

    printf("    .text\n");
    genFuns(p);

    printf("    .data\n");
    return 0;
}
