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
//int paramCount = 0;

//Struct
struct Entry {
    struct Entry *next;
    char *name;
};

//Global struct
struct Entry *table;

/*int get(char *id) {
    struct Entry *head = table;
    while (head != NULL) {
        if (strcmp(head -> name, id) == 0) {
            return 1;
        }
        head = head -> next;
    }
    return 0;
}*/

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

int formal(Fun * p, char * s) {
    if (p != NULL) {
        if (p -> formals != NULL) {
            Formals *myFormal = p -> formals;
            for (int i = 0; i < p -> formals -> n; i++) {
                if (strcmp(s, myFormal -> first) == 0) {
                    return (p -> formals -> n) - (myFormal -> n) + 1;
                }
                myFormal = myFormal -> rest;
            }
        }
    }
    return -1;
}

void myExpression (Expression * e, Fun * p) {
    if (e != NULL) {
    switch (e -> kind) {
        case eVAR : {
            int inside = formal(p, e -> varName);
            if (inside == -1) {
                set(e -> varName);
                printf("    mov %s, %%r15\n", e -> varName);
            }
            else {
                printf("    mov %d(%%rbp), %%r15\n", 8 * (inside + 1));
            }
            break;
        }
        case eVAL : {
            printf("    push %%r13\n");
            printf("    mov $");
            printf("%lu", e -> val);
            printf(", %%r13\n");
            printf("    mov %%r13, %%r15\n");
            printf("    pop %%r13\n");
            break;
        }
        case ePLUS : {
            printf("    push %%r13\n");
            myExpression(e -> left, p);
            printf("    pop %%r13\n");
            printf("    mov %%r15, %%r13\n");
            printf("    push %%r13\n");
            myExpression(e -> right, p);
            printf("    pop %%r13\n");
            printf("    add %%r13, %%r15\n");
            break;
        }
        case eMUL : {
            printf("    push %%r13\n");
            myExpression(e -> left, p);
            printf("    pop %%r13\n");
            printf("    mov %%r15, %%r13\n");
            printf("    push %%r13\n");
            myExpression(e -> right, p);
            printf("    pop %%r13\n");
            printf("    imul %%r13, %%r15\n");
            break;
        }
        case eEQ : {
            myExpression(e -> left, p);
            printf("    mov %%r15, %%r13\n");
            myExpression(e -> right, p);
            printf("    cmp %%r13, %%r15\n");
            printf("    setz %%r15b\n");
            printf("    movzx %%r15b, %%r15\n"); 
            break;
        }
        case eNE : {
            myExpression(e -> left, p);
            printf("    mov %%r15, %%r13\n");
            myExpression(e -> right, p);
            printf("    cmp %%r13, %%r15\n");
            printf("    setnz %%r15b\n");
            printf("    movzx %%r15b, %%r15\n"); 
            break;
        }
        case eLT : {
            myExpression(e -> left, p);
            printf("    mov %%r15, %%r13\n");
            myExpression(e -> right, p);
            printf("    cmp %%r15, %%r13\n");
            printf("    setl %%r15b\n");
            printf("    movzx %%r15b, %%r15\n");
            break;
        }
        case eGT : {
            myExpression(e -> left, p);
            printf("    mov %%r15, %%r13\n");
            myExpression(e -> right, p);
            printf("    cmp %%r15, %%r13\n");
            printf("    setg %%r15b\n");
            printf("    movzx %%r15b, %%r15\n");
            break;
        }
        case eCALL : {
            if (e != NULL) {
            if (e -> callActuals != NULL) {
            for (int i = e -> callActuals -> n - 1; i >= 0; i--) {
                Actuals *actual = e -> callActuals; 
                for (int a = 0; a < i; a++) {
                    actual = actual -> rest;
                }
                myExpression(actual -> first, p);
                printf("    push %%r15\n");
            }
            }
            }
            printf("    call ");
            printf("%s\n", e -> callName);
            if (e != NULL) {
            if (e -> callActuals != NULL) {
            for (int i = 0; i < e -> callActuals -> n; i++) {
                printf("    pop %%r14\n");
            }
            }
            }
            break;
        }
        default : {
            break;
        }
    }
    }
}

void myStatement(Statement * s, Fun * p) {
    if (s != NULL) {
    switch (s -> kind) {
        case sAssignment : {
            myExpression(s -> assignValue, p);
            int inside = formal(p, s -> assignName);
            if (inside == -1) {
                set(s -> assignName);
                printf("    mov %%r15, ");
                printf("%s\n", s -> assignName);
            }
            else {
                printf("    mov %%r15, %d(%%rbp)\n", 8 * (inside + 1));
            }
            break;
        }
        case sPrint : {
            myExpression(s -> printValue, p);
            printf("    mov $p4_format, %%rdi\n");
            printf("    mov %%r15, %%rsi\n");
            printf("    mov $0, %%rax\n");
            printf("    call printf\n");
            break;
        }
        case sIf : {
            myExpression(s -> ifCondition, p);
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
            myExpression(s -> whileCondition, p);
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
            myExpression(s -> returnValue, p);
            printf("    mov %%rbp, %%rsp\n");
            printf("    pop %%rbp\n");
            printf("    mov $0,%%rax\n");
            printf("    ret\n");
            break;
        } 
        default : {
            break;
        }
    }
    }
}

void genFun(Fun * p) {
    printf("    .global %s\n", p -> name);
    printf("%s:\n", p -> name);
    printf("    push %%rbp\n");
    printf("    mov %%rsp, %%rbp\n");
    myStatement(p -> body, p);
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
