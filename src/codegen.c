#include "mdcc.h"

int size_of(Type *ty) {
    if(ty->kind == INT) {
        return 4;
    } else if(ty->kind == PTR) {
        return 8;
    }
    return 0;
}

// Code generation
void gen_lval(Node *node) {
    if(node == NULL) return;
    switch(node->kind) {
        case ND_LVAR:
            printf("    lea rax, [rbp-%d]\n", node->lvar->offset);
            printf("    push rax\n");
            return;
        case ND_DEREF:
            gen(node->lhs);
            return;
    }
/*     printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node->offset - 8);
    printf("    push rax\n"); */
}



void gen_fun(Func *func) {
    printf("#FUN\n");
    {
        int cnt = 0;
        char *reg[6] = {"RDI", "RSI", "RDX", "RCX", "R8", "R9"};
        Arg *now = func->arg;
        while(now) {
            cnt++;
            now = now->next;
        }
        // prologue
        printf("%.*s:\n", func->len, func->name);
        printf("    push rbp\n");
        printf("    mov rbp, rsp\n");
        //fprintf(stderr, "%d\n", locals->offset);
        
        printf("    sub rsp, %d\n", func->depth);
        for(int i=0;i<cnt;++i) {
            printf("    mov [rbp-%d], %s\n", (i + 1) * 8, reg[i]);
            //printf("    push %s\n", reg[i]);
        }

        // generate

        cnt = 0;
        while(func->children[cnt]) {
            gen(func->children[cnt]);
            cnt++;
        }
        return;
    }
}

void gen(Node *node) {
    if(node == NULL) return;
    switch(node->kind) {
        case ND_NULL:
            return;
        case ND_NUM:
            printf("#NUM\n");
            printf("    push %d\n", node->val);
            return;
        case ND_LVAR:
            printf("#LVAR\n");
            gen_lval(node);
            printf("    pop rax\n");

            if(node->ty->kind == INT) {
                printf("    movsx rax, dword ptr [rax]\n");
            } else if(node->ty->kind == PTR) {
                printf("mov rax, [rax]\n");
            } else {
                error("node->ty must be INT or PTR\n");
            } 


            printf("    push rax\n");
            return;
        case ND_ASSIGN:
            printf("#ASSIGN\n");
            gen_lval(node->lhs);
            gen(node->rhs);

            printf("    pop rdi\n");
            printf("    pop rax\n");
            if(node->ty->kind == INT) {
                printf("    mov [rax], edi\n");
            } else if(node->ty->kind == PTR) {
                printf("    mov [rax], rdi\n");
            } else {
                error("node->ty must be INT or PTR\n");
            }
            printf("    push rdi\n");
            return;
        case ND_RETURN:
            printf("#RETURN\n");
            gen(node->lhs);
            printf("    pop rax\n");
            printf("    mov rsp, rbp\n");
            printf("    pop rbp\n");
            printf("    ret\n");
            return;
        case ND_IF:
            {
                int label = fresh_id();
                gen(node->children[0]);
                printf("    pop rax\n");
                printf("    cmp rax, 0\n");
                if(node->children[2]) printf("    je .Lelse%d\n", label);
                else printf("   je .Lend%d\n", label);
                gen(node->children[1]);
                if(node->children[2]) {
                    printf("    jmp .Lend%d\n", label);
                    printf(".Lelse%d:\n", label);
                    gen(node->children[2]);
                }
                printf(".Lend%d:\n", label);
                return;
            }
        case ND_WHILE:
            {
                int label = fresh_id();
                printf(".Lbegin%d:\n", label);
                gen(node->children[0]);
                printf("    pop rax\n");
                printf("    cmp rax, 0\n");
                printf("    je .Lend%d\n", label);
                gen(node->children[1]);
                printf("    jmp .Lbegin%d\n", label);
                printf(".Lend%d:\n", label);
                return;
            }
        case ND_FOR:
            {
                int label = fresh_id();
                gen(node->children[0]);
                printf(".Lbegin%d:\n", label);
                gen(node->children[1]);
                printf("    pop rax\n");
                printf("    cmp rax, 0\n");
                printf("    je .Lend%d\n", label);
                gen(node->children[3]);
                gen(node->children[2]);
                printf("    jmp .Lbegin%d\n", label);
                printf(".Lend%d:\n", label);
                return;
            }
        case ND_BLOCK:
            for(int i=0;node->children[i];++i) {
                gen(node->children[i]);
                if(node->children[i+1]) printf("    pop rax\n");
            }
            return;
        case ND_APP:
            printf("#APP\n");
            {
                int cnt = 0;
                char *reg[6] = {"RDI", "RSI", "RDX", "RCX", "R8", "R9"};

                for(int i=0;node->children[i];++i) {
                    gen(node->children[i]);
                    cnt++;
                }

                while(cnt) {
                    cnt--;
                    printf("    pop %s\n", reg[cnt]);
                }
                // prologue
                // printf("    push rbp\n");
                // printf("    mov rbp, rsp\n");
                // printf("    sub rsp, %d\n", (cnt + cnt % 2) * 8);
                // call
                printf("    call %.*s\n", node->lvar->len, node->lvar->name);
                printf("    push rax\n");
                // epilogue
                // printf("    pop rax\n");
                // printf("    mov rsp, rbp\n");
                // printf("    pop rbp\n");
                // printf("    ret\n");
                return;
            }
        case ND_ADDR:
            printf("#ADDR\n");
            gen_lval(node->lhs);
            return;
        case ND_DEREF:
            printf("#DEREF\n");
            gen(node->lhs);
            printf("    pop rax\n");
            printf("    mov rax, [rax]\n");
            printf("    push rax\n");
            return;

        case ND_SIZEOF:
            printf("#SIZEOF\n");
            printf("    push %d\n", node->size);
            return;
        default:
            break;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("#BINOP\n");
    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch(node->kind) {
        case ND_ADD:
            printf("#ADD\n");
            if(node->lhs->ty->kind == PTR) {
                printf("    imul rdi, %d\n", size_of(node->rhs->ty));
            }
            printf("    add rax, rdi\n");
            break;
        case ND_SUB:
            printf("#SUB\n");

            printf("    sub rax, rdi\n");
            break;
        case ND_MUL:
            printf("#MUL\n");
            printf("    imul rax, rdi\n");
            break;
        case ND_DIV:
            printf("#DIV\n");
            printf("    cqo\n");
            printf("    idiv rdi\n");
            break;
        case ND_BEQ:
            printf("#BEQ\n");
            printf("    cmp rax, rdi\n");
            printf("    sete al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_NEQ:
            printf("#NEQ\n");
            printf("    cmp rax, rdi\n");
            printf("    setne al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_LT:
            printf("#LT\n");
            printf("    cmp rax, rdi\n");
            printf("    setl al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_LE:
            printf("#LE\n");
            printf("    cmp rax, rdi\n");
            printf("    setle al\n");
            printf("    movzb rax, al\n");
            break;
        default:
            error("Can't generate code. node->kind is %s", enum2str(node->kind));
            break;
    }

    printf("    push rax\n");
}