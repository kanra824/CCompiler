#include "mdcc.h"

Token *token; // token sequence
char *user_input; // program input
LVar *locals; // local_variables;
Top *code[100]; // node sequence
int id;
int toplevel = 1;
Tyenv *tyenv, *tyenv_fun;
int cntptr_ty = 0;
GVar *globals;

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

    int i = 0;
    while(code[i]) {
        if(code[i]->gvar == NULL) {
            int offset = 0;
            LVar *now = code[i]->lvar;
            while(now) {
                offset += size_of(now->ty);
                now->offset = offset;
                now = now->next;
            }
            code[i]->depth = offset;
        }
        i++;
    }
    
    int global_offset = 0;
    GVar *gvar = globals;
    while(gvar) {
        global_offset += size_of(gvar->ty);
        gvar->offset = global_offset;
        gvar = gvar->next;
    }


    tyenv = calloc(1, sizeof(Tyenv));
    tyenv_fun = calloc(1, sizeof(Tyenv));
    tyenv_fun->str = "";
    i = 0;
    while(code[i]) {
        Tyenv *newenv = calloc(1, sizeof(Tyenv));
        tyenv = newenv;
        tycheck_fun(code[i++]);
    }

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf(".global alloc4\n");

    GVar *now = globals;
    while(now) {
        printf("    .data\n");
        printf("%.*s:\n", now->len, now->name);
        printf("    .zero %d\n", now->offset);
        now = now->next;
    }

    printf("    .text\n");
    // generate code in order
    for(int i=0;code[i];++i) {
        gen_top(code[i]);
    }
    return 0;
}