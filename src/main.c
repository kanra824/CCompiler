#include "mdcc.h"

Token *token;
char *user_input;

int main(int argc, char **argv) {
    if(argc != 2) {
        error("The number of arguments are not valid.");
        return 1;
    }

    user_input = argv[1];
    
    token = tokenize(user_input);
    #ifdef DEBUG
        printf("print tokens\n");
        print_tokens(token);
    #endif
    
    Node *node = expr();
    #ifdef DEBUG
        printf("print nodes\n");
        print_nodes(node, 0);
    #endif

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // generate code
    gen(node);

    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}