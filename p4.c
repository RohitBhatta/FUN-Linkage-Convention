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
Fun *func;

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
}

int formals(Fun * p, char * s) {
    Formals *myFormal = p -> formals;
    for (int i = 0; i < p -> formals -> n; i++) {
         if (strcmp(s, myFormal -> first) == 0) {
             return ((p -> formals -> n) - (myFormal -> n) + 1);
         }
         myFormal = myFormal -> rest;
    }
    return -1;
}

void myExpression (Expression * e) {
    switch (e -> kind) {
        /*case eVAR : {
            printf("    push %%r13\n");
            printf("    mov $");
            printf("%s", e -> varName);
            printf(", %%r13\n");
            printf("    mov %%r13, %%r15\n");
            printf("    pop %%r13\n");
            break;
        }*/
        case eVAR : {
            printf("    push %%r15\n");
            printf("    push %%r13\n");
            int inside = formals(func, e -> varName);
            if (inside == -1) {
                printf("    mov %%r15, %d(%%rbp)", 8 * (inside + 1));
            }
            else {
                set(e -> varName);
                printf("    mov %%r15, ");
                printf("%s\n", e -> varName);
            }
            printf("    pop %%r13\n");
            printf("    pop %%r15\n");
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
            for (int i = paramCount - 1; i >= 0; i--) {
                Actuals *actual = e -> callActuals; 
                for (int a = 0; a < i; a++) {
                    actual = actual -> rest;
                }
                myExpression(actual -> first);
                printf("    push %%r15\n");
            }
            printf("    call ");
            printf("%s\n", e -> callName);
            for (int i = 0; i < paramCount; i++) {
                printf("    pop %%r15\n");
            }
            paramCount = 0;
            break;
        }
        default : {
            break;
        }
    }
}

void myStatement(Statement * s, Fun * p) {
    switch (s -> kind) {
        case sAssignment : {
            printf("    push %%r15\n");
            func = p;
            myExpression(s -> assignValue);
            int inside = formals(p, s -> assignName);
            if (inside == -1) {
                set(s -> assignName);
                printf("    mov %%r15, ");
                printf("%s\n", s -> assignName);
            }
            else {
                printf("    mov %%r15, %d(%%rbp)", 8 * (inside + 1));
            }
            printf("    pop %%r15\n");
            break;
        }
        case sPrint : {
            printf("    push %%r15\n");
            myExpression(s -> printValue);
            printf("    mov $p4_format, %%rdi\n");
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
            myStatement(s -> ifThen, p);
            printf("%s%d\n", "    jmp complete", completeTemp);
            printf("%s%d%s\n", "    else", elseTemp, ":");
            myStatement(s -> ifElse, p);
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
            myStatement(s -> whileBody, p);
            printf("%s%d\n", "    jmp again", againTemp);
            printf("%s%d%s\n", "    finished", finishedTemp, ":");
            break;
        } 
        case sBlock : {
            Block *current = s -> block;
            while (current != NULL) {
                myStatement(current -> first, p);
                current = current -> rest;
            }
            break;
        } 
        case sReturn : {
            myExpression(s -> returnValue);
            printf("    ret\n");
            break;
        } 
        default : {
            break;
        }
    }
}

void genFun(Fun * p) {
    printf("    .global %s\n", p -> name);
    printf("%s:\n", p -> name);
    if (p -> formals != NULL) {
        Formals *param = p -> formals;
        paramCount = param -> n;
    }
    printf("    push %%rbp\n");
    printf("    mov %%rsp, %%rbp\n");
    printf("    push %%r13\n");
    printf("    push %%r14\n");
    printf("    push %%r15\n");
    myStatement(p -> body, p);
    printf("    pop %%r15\n");
    printf("    pop %%r14\n");
    printf("    pop %%r13\n");
    printf("    mov %%rbp, %%rsp\n");
    printf("    pop %%rbp\n");
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
    table = (struct Entry *) malloc(sizeof(struct Entry));
    genFuns(p);

    printf("    .data\n");
    printf("p4_format: .string\"%%d\\n\"\n");
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
