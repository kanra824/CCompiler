#include "mdcc.h"

// Code generation
void gen_lval(Node *node) {
    if(node == NULL) return;
    if(node->kind != ND_LVAR) {
        error("lvalue of substitution is not a variable");
    }
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node->offset);
    printf("    push rax\n");
}

void gen(Node *node) {
    if(node == NULL) return;
    switch(node->kind) {
        case ND_NUM:
            printf("    push %d\n", node->val);
            return;
        case ND_LVAR:
            gen_lval(node);
            printf("    pop rax\n");
            printf("    mov rax, [rax]\n");
            printf("    push rax\n");
            return;
        case ND_ASSIGN:
            gen_lval(node->lhs);
            gen(node->rhs);

            printf("    pop rdi\n");
            printf("    pop rax\n");
            printf("    mov [rax], rdi\n");
            printf("    push rdi\n");
            return;
        case ND_RETURN:
            gen(node->lhs);
            printf("    pop rax\n");
            printf("    mov rsp, rbp\n");
            printf("    pop rbp\n");
            printf("    ret\n");
            return;
        case ND_IF:
            {
                int Lelse = fresh_id(), Lend = fresh_id();
                gen(node->children[0]);
                printf("    pop rax\n");
                printf("    cmp rax, 0\n");
                if(node->children[2]) printf("    je .Lelse%d\n", Lelse);
                else printf("   je .Lend%d\n", Lend);
                gen(node->children[1]);
                if(node->children[2]) {
                    printf("    jmp .Lend%d\n", Lend);
                    printf(".Lelse%d:\n", Lelse);
                    gen(node->children[2]);
                }
                printf(".Lend%d:\n", Lend);
                return;
            }
        case ND_WHILE:
            {
                int Lbegin = fresh_id(), Lend = fresh_id();
                printf(".Lbegin%d:\n", Lbegin);
                gen(node->children[0]);
                printf("    pop rax\n");
                printf("    cmp rax, 0\n");
                printf("    je .Lend%d\n", Lend);
                gen(node->children[1]);
                printf("    jmp .Lbegin%d\n", Lbegin);
                printf(".Lend%d:\n", Lend);
                return;
            }
        case ND_FOR:
            {
                int Lbegin = fresh_id(), Lend = fresh_id();
                gen(node->children[0]);
                printf(".Lbegin%d:\n", Lbegin);
                gen(node->children[1]);
                printf("    pop rax\n");
                printf("    cmp rax, 0\n");
                printf("    je .Lend%d\n", Lend);
                gen(node->children[3]);
                gen(node->children[2]);
                printf("    jmp .Lbegin%d\n", Lbegin);
                printf(".Lend%d:\n", Lend);
                return;
            }
        case ND_BLOCK:
            for(int i=0;node->children[i];++i) {
                gen(node->children[i]);
                if(node->children[i+1]) printf("    pop rax\n");
            }
            return;
        case ND_APP:
            {
                int cnt = 0;
                char *reg[6] = {"RDI", "RSI", "RDX", "RCX", "R8", "R9"};
                for(int i=0;node->children[i];++i) {
                    gen(node->children[i]);
                    printf("    pop rax\n");
                    printf("    mov %s, rax\n", reg[i]);
                    ++cnt;
                }
                // prologue
                // printf("    push rbp\n");
                // printf("    mov rbp, rsp\n");
                // printf("    sub rsp, %d\n", (cnt + cnt % 2) * 8);
                // call
                printf("    call %.*s\n", node->len, node->str);
                printf("    push rax\n");
                // epilogue
                // printf("    pop rax\n");
                // printf("    mov rsp, rbp\n");
                // printf("    pop rbp\n");
                // printf("    ret\n");
                return;
            }
        case ND_FUN:
            {
                int cnt = 0;
                char *reg[6] = {"RDI", "RSI", "RDX", "RCX", "R8", "R9"};
                for(int i=0;node->children[i+1];++i){
                    ++cnt;
                }
                // prologue
                printf("%.*s:\n", node->len, node->str);
                printf("    push rbp\n");
                printf("    mov rbp, rsp\n");
                printf("    sub rsp, %d\n", (cnt + cnt % 2) * 8);
                for(int i=0;i<cnt;++i) {
                    printf("    mov [rbp-%d], %s\n", (i+1) * 8, reg[i]);
                }

                // generate
                gen(node->children[cnt]);

                return;
            }
        default:
            break;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch(node->kind) {
        case ND_ADD:
            printf("    add rax, rdi\n");
            break;
        case ND_SUB:
            printf("    sub rax, rdi\n");
            break;
        case ND_MUL:
            printf("    imul rax, rdi\n");
            break;
        case ND_DIV:
            printf("    cqo\n");
            printf("    idiv rdi\n");
            break;
        case ND_BEQ:
            printf("    cmp rax, rdi\n");
            printf("    sete al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_NEQ:
            printf("    cmp rax, rdi\n");
            printf("    setne al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_LT:
            printf("    cmp rax, rdi\n");
            printf("    setl al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_LE:
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