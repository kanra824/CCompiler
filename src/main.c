#include "mdcc.h"

Token *token; // token sequence
char *user_input; // program input
LVar *locals; // local_variables;
Node *code[100]; // node sequence
int id;
int toplevel = 1;
Tyenv *tyenv, *tyenv_fun;

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
        }
    #endif

    tyenv = calloc(1, sizeof(Tyenv));
    tyenv_fun = calloc(1, sizeof(Tyenv));
    tyenv_fun->str = "";
    int i = 0;
    while(code[i]) {
        Tyenv *newenv = calloc(1, sizeof(Tyenv));
        tyenv = newenv;
        tycheck(code[i++]);
    }

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");

    // generate code in order
    for(int i=0;code[i];++i) {
        gen(code[i]);
    }
    return 0;
}