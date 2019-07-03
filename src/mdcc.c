#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// kind of token
typedef enum {
    TK_RESERVED, // symbol
    TK_NUM, // integer token
    TK_EOF, // end of file
} TokenKind;

typedef struct Token Token;

// token type
struct Token {
    TokenKind kind; // type of token
    Token *next; // next token
    int val; // token value(if kind == TK_NUM)
    char *str; // token string
};

// kind of AST node
typedef enum {
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_NUM, // 整数
} NodeKind;

// type of AST Node
typedef struct Node Node;
struct Node {
    NodeKind kind; // type of node
    Node *lhs; // left child
    Node *rhs; // right child
    int val; // use if kind == ND_NUM
};

//---------------------------------------------------------------
// Function prototype

// Error function
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

// Lexer
bool consume(char op);
void expect(char op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str);
Token *tokenize(char *p);

// Parser
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *expr();
Node *mul();
Node *unary();
Node *term();

// Code generation
void gen(Node *node);

//---------------------------------------------------------------
// Error function

// report error
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// program input
char *user_input;
// report error and where it is occured
void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}


// -------------------------------------------------
// Lexer


// The token we are looking at
Token *token;

/* if next token is expected to be a symbol
 * then read next token and return true
 * else return false
 */
bool consume(char op) {
    if(token->kind != TK_RESERVED || token->str[0] != op) {
        return false;
    }
    token = token->next;
    return true;
}

/* if next token is expected symbol
 * then read next token
 * else report error
 */
void expect(char op) {
    if(token->kind != TK_RESERVED || token->str[0] != op) {
        error_at(token->str, "It's not '%c'", op);
    }
    token = token->next;
}

int expect_number() {
    if(token->kind != TK_NUM) {
        error_at(token->str, "It's not number");
    }
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() {
    return token->kind == TK_EOF;
}

// create new token and connect to cur
Token *new_token(TokenKind kind, Token *cur, char *str) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

// tokenize p and return it
Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while(*p) {
        // skip space
        if(isspace(*p)) {
            p++;
            continue;
        }

        if(*p == '+' || *p == '-' || *p == '*' || *p == '/' ||
           *p == '(' || *p == ')') {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }

        if(isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error_at(p, "Cant tokenize");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Parser

/*
 * expr = mul ("+" mul | "-" mul)*
 * mul = term ("*" term | "/" term)*
 * term = num | "(" expr ")"
 */


Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

Node *expr() {
    Node *node = mul();
    for(;;) {
        if(consume('+')) {
            node = new_node(ND_ADD, node, mul());
        } else if(consume('-')) {
            node = new_node(ND_SUB, node, mul());
        } else {
            return node;
        }
    }
}

Node *mul() {
    Node *node = unary();
    for(;;) {
        if(consume('*')) {
            node = new_node(ND_MUL, node, unary());
        } else if(consume('/')) {
            node = new_node(ND_DIV, node, unary());
        } else {
            return node;
        }
    }
}

Node *unary() {
    if(consume('+')) {
        return term();
    } else if(consume('-')) {
        return new_node(ND_SUB, new_node_num(0), term());
    } else {
        return term();
    }
}

Node *term() {
    // if next_token == '(' then term must be '(' expr ')'
    if(consume('(')) {
        Node *node = expr();
        expect(')');
        return node;
    }
    // else term muse be a number
    return new_node_num(expect_number());
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Code generation


void gen(Node *node) {
    if(node->kind == ND_NUM) {
        printf("    push %d\n", node->val);
        return;
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
        default:
            error("Can't generate code. This node must be some operator.");
            break;
    }

    printf("    push rax\n");
}

//-------------------------------------------------
// main

int main(int argc, char **argv) {
    if(argc != 2) {
        error("The number of arguments are not valid.");
        return 1;
    }

    user_input = argv[1];
    token = tokenize(user_input);
    Node *node = expr();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // generate code
    gen(node);

    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}
