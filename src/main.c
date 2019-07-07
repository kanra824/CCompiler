#include "mdcc.h"

int main(int argc, char **argv) {
    if(argc != 2) {
        error("The number of arguments are not valid.");
        return 1;
    }

    user_input = argv[1];
    locals = calloc(1, sizeof(LVar));
    locals->offset = 0;
    
    token = tokenize(user_input);
    #ifdef DEBUG
        printf("print tokens\n");
        print_tokens(token);
    #endif

    program();
    #ifdef DEBUG
        printf("print nodes\n");
        
        for(int i=0;code[i];++i) {
            print_nodes(code[i], 0);
            printf("\n\n");
        }
    #endif

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // prologue
    // Allocate space for variables
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, 208\n");

    // generate code in order
    for(int i=0;code[i];++i) {
        gen(code[i]);
        printf("    pop rax\n");
    }

    // epilogue
    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
    return 0;
}