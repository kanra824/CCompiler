#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// A value expressing token
enum {
    TK_NUM = 256, // integer
    TK_EOF, // end of file
};

// type of token
typedef struct {
    int ty; // type of the token
    int val; // the value of TK_NUM
    char *input; // token string(for an error message)
} Token;

// program
char *user_input;

// Token sequense
Token tokens[100];

// error function
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// Report where the error occur
void error_at(char *loc, char *msg) {
    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, ""); // output 'blank' pos times
    fprintf(stderr, "^ %s\n", msg);
    exit(1);
}

// tokenizer
void tokenize() {
    char *p = user_input;

    int i = 0;
    while(*p) {
        if(isspace(*p)) {
            ++p;
            continue;
        }

        if(*p == '+' || *p == '-') {
            tokens[i].ty = *p;
            tokens[i].input = p;
            ++i;
            ++p;
            continue;
        }

        if(isdigit(*p)) {
            tokens[i].ty = TK_NUM;
            tokens[i].input = p;
            tokens[i].val = strtol(p, &p, 10);
            ++i;
            continue;
        }

        error_at(p, "Can't tokenize.");
    }

    tokens[i].ty = TK_EOF;
    tokens[i].input = p;
}

int main(int argc, char **argv) {
    if(argc != 2) {
        error("The number of arguments is not valid");
        return 1;
    }

    // tokenize
    user_input = argv[1];
    tokenize();

    // output top of the assembly
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // Check whether the first part of the expression is number or not.
    // output mov
    if(tokens[0].ty != TK_NUM) {
        error_at(tokens[0].input, "This is not number");
    }
    printf("    mov rax, %d\n", tokens[0].val);

    // Processiong "+ <number>" and "- <number>"
    int i = 1;
    while(tokens[i].ty != TK_EOF) {
        if(tokens[i].ty == '+') {
            ++i;
            if (tokens[i].ty != TK_NUM) {
                error_at(tokens[i].input, "This is not number");
            }
            printf("    add rax, %d\n", tokens[i].val);
            ++i;
            continue;
        }

        if(tokens[i].ty == '-') {
            ++i;
            if(tokens[i].ty != TK_NUM) {
                error_at(tokens[i].input, "This is not number");
            }
            printf("    sub rax, %d\n", tokens[i].val);
            ++i;
            continue;
        }
        
        error_at(tokens[i].input, "Can't interpret");
    }

    printf("    ret\n");
    return 0;
}