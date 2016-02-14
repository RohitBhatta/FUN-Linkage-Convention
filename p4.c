#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

int tableCount = 0;
int first = 0;
int elseCount = 0;
int completeCount = 0;
int againCount = 0;
int finishedCount = 0;
int paramCount = 0;

//Struct
struct Entry {
    struct Entry *next;
    char *name;
};

//Global struct
struct Entry *table;

int get(char *id) {
    struct Entry *head = table;
    while (head != NULL) {
        if (strcmp(head -> name, id) == 0) {
            return 1;
        }
        head = head -> next;
    }
    return 0;
}

void set(char *id) {
    int same = 0;
    struct Entry *current = (struct Entry *) malloc(sizeof(struct Entry));
    if (first == 0) {
        table -> name = id;
        table -> next = NULL;
        first++;
        tableCount++;
    }
    else {
        current = table;
        while (current -> next != NULL) {
            if (strcmp(current -> name, id) == 0) {
                same = 1;
            }
            current = current -> next;
        }
        if (strcmp(current -> name, id) == 0) {
            same = 1;
        }
    }
    if (!same) {
        current -> next = malloc(sizeof(struct Entry));
        //No memory left
        if (current -> next == NULL) {
            printf("Out of memory\n");
        }
        //Assign and print values
        else {
            current = current -> next;
            current -> name = id;
            current -> next = NULL;
            tableCount++;
        }
    }
    //printf("    mov %%r15, ");
    //printf("%s\n", id);
}

void myExpression (Expression * e) {
    switch (e -> kind) {
        case eVAR : {
            printf("    push %%r13\n");
            printf("    mov $");
            printf("%s", e -> varName);
            printf(", %%r13\n");
            printf("    mov %%r13, %%r15\n");
            printf("    pop %%r13\n");
            break;
        }
        case eVAL : {
            printf("    push %%r13\n");
            printf("    mov $");
            printf("%ld", e -> val);
            printf(", %%r13\n");
            printf("    mov %%r13, %%r15\n");
            printf("    pop %%r13\n");
            break;
        }
        case ePLUS : {
            printf("    push %%r13\n");
            myExpression(e -> left);
            printf("    pop %%r13\n");
            printf("    mov %%r15, %%r13\n");
            printf("    push %%r13\n");
            myExpression(e -> right);
            printf("    pop %%r13\n");
            printf("    add %%r13, %%r15\n");
            break;
        }
        case eMUL : {
            printf("    push %%r13\n");
            myExpression(e -> left);
            printf("    pop %%r13\n");
            printf("    mov %%r15, %%r13\n");
            printf("    push %%r13\n");
            myExpression(e -> right);
            printf("    pop %%r13\n");
            printf("    imul %%r13, %%r15\n");
            break;
        }
        case eEQ : {
            myExpression(e -> left);
            printf("    mov %%r15, %%r13\n");
            myExpression(e -> right);
            printf("    cmp %%r13, %%r15\n");
            printf("    setz %%r15b\n");
            printf("    movzx %%r15b, %%r15\n"); 
            break;
        }
        case eNE : {
            myExpression(e -> left);
            printf("    mov %%r15, %%r13\n");
            myExpression(e -> right);
            printf("    cmp %%r13, %%r15\n");
            printf("    setnz %%r15b\n");
            printf("    movzx %%r15b, %%r15\n"); 
            break;
        }
        case eLT : {
            myExpression(e -> left);
            printf("    mov %%r15, %%r13\n");
            myExpression(e -> right);
            printf("    cmp %%r15, %%r13\n");
            printf("    setl %%r15b\n");
            printf("    movzx %%r15b, %%r15\n");
            break;
        }
        case eGT : {
            myExpression(e -> left);
            printf("    mov %%r15, %%r13\n");
            myExpression(e -> right);
            printf("    cmp %%r15, %%r13\n");
            printf("    setg %%r15b\n");
            printf("    movzx %%r15b, %%r15\n");
            break;
        }
        case eCALL : {
            //Push actuals onto stack
            //Maybe have if statement to check for all conditions
            //Find way to offset or push registers back to preserve stack alignment
            Actuals *actual = e -> callActuals;
            for (int i = 0; i < paramCount; i++) {
                printf("    push %%r15\n");
                myExpression(actual -> first);
                actual = actual -> rest;
            }
            printf("    call ");
            printf("%s\n", e -> callName);
            paramCount = 0;
            break;
        }
        default : {
            break;
        }
    }
}

void myStatement(Statement * s) {
    switch (s -> kind) {
        case sAssignment : {
            printf("    push %%r15\n");
            //printf("    mov ");
            myExpression(s -> assignValue);
            set(s -> assignName);
            printf("    pop %%r15\n");
            break;
        }
        case sPrint : {
            printf("    push %%r15\n");
            myExpression(s -> printValue);
            printf("    mov $p3_format, %%rdi\n");
            printf("    mov %%r15, %%rsi\n");
            printf("    mov $0, %%rax\n");
            printf("    call printf\n");
            printf("    pop %%r15\n");
            break;
        }
        case sIf : {
            myExpression(s -> ifCondition);
            int elseTemp = elseCount;
            int completeTemp = completeCount;
            elseCount++;
            completeCount++;
            printf("    cmp $0, %%r15\n");
            printf("%s%d\n", "    je else", elseTemp);
            myStatement(s -> ifThen);
            printf("%s%d\n", "    jmp complete", completeTemp);
            printf("%s%d%s\n", "    else", elseTemp, ":");
            myStatement(s -> ifElse);
            printf("%s%d%s\n", "    complete", completeTemp, ":");
            break;
        } 
        case sWhile : {
            int againTemp = againCount;
            int finishedTemp = finishedCount;
            againCount++;
            finishedCount++;
            printf("%s%d%s\n", "    again", againTemp, ":");
            myExpression(s -> whileCondition);
            printf("    cmp $0, %%r15\n");
            printf("%s%d\n", "    je finished", finishedTemp);
            myStatement(s -> whileBody);
            printf("%s%d\n", "    jmp again", againTemp);
            printf("%s%d%s\n", "    finished", finishedTemp, ":");
            break;
        } 
        case sBlock : {
            Block *current = s -> block;
            while (current != NULL) {
                myStatement(current -> first);
                current = current -> rest;
            }
            break;
        } 
        case sReturn : {
            printf("    mov %%r15, %%rax\n");
            break;
        } 
        default : {
            break;
        }
    }
}

void genFun(Fun * p) {
    table = (struct Entry *) malloc(sizeof(struct Entry));
    printf("    .global %s\n", p -> name);
    printf("%s:\n", p -> name);
    if (p -> formals != NULL) {
        Formals *param = p -> formals;
        paramCount = param -> n;
    }
    printf("    push %%r13\n");
    printf("    push %%r14\n");
    printf("    push %%r15\n");
    myStatement(p -> body);
    printf("    pop %%r15\n");
    printf("    pop %%r14\n");
    printf("    pop %%r13\n");
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
    printf("p3_format: .string\"%%d\\n\"\n");
    if (tableCount != 0) {
        while (table -> next != NULL) {
            printf("    ");
            printf("%s", table -> name);
            printf(": .quad 0\n");
            table = table -> next;
        }
        printf("    ");
        printf("%s", table -> name);
        printf(": .quad 0\n");
        table = table -> next;
    }
    return 0;
}