#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #define DEBUG

// kind of token
typedef enum {
    TK_RESERVED, // symbol
    TK_IDENT, // identifier
    TK_NUM, // integer token
    TK_EOF, // end of file
} TokenKind;

// token type
typedef struct Token Token;
struct Token {
    TokenKind kind; // type of token
    Token *next; // next token
    int val; // token value(if kind == TK_NUM)
    char *str; // token string
    int len; // token length
};

// Node type
typedef struct Type Type;
struct Type {
    char *str;
    int len;
    enum { LVAL, RVAL, DEC } val;
    enum { INT, PTR, FUN } kind;
    Type *param[100];
    Type *ret;
    Type *ptr_to;
};

typedef struct Tyenv Tyenv;
struct Tyenv {
    char *str;
    int len;
    Type ty;
    Tyenv *ptr_to;
};

typedef struct Env Env;
struct Env {
    char *str;
    int len;
    Env *ptr_to;
};

typedef struct LVar LVar;
// type of local variable
struct LVar {
    LVar *next; // next variable or NULL
    char *name; // name of variable
    int len; // length of name
    int offset; // offset from RBP
    Type *ty;
};

// kind of AST node
typedef enum {
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_BEQ, // ==
    ND_NEQ, // !=
    ND_LT, // <
    ND_LE, // <=
    ND_ASSIGN, // =
    ND_LVAR, // local variable
    ND_RETURN, // return
    ND_IF, // if
    ND_WHILE, // while
    ND_FOR, // for
    ND_BLOCK, // block
    ND_APP, // apply
    ND_ADDR, // &
    ND_DEREF, // *
    ND_NUM, // 整数
    ND_NULL
} NodeKind;

// AST Node
typedef struct Node Node;
struct Node {
    NodeKind kind; // type of node
    Node *lhs; // left child
    Node *rhs; // right child
    Node *children[100]; // children
    Type *ty; // type
    int val; // use if kind == ND_NUM
    char *str;
    int len;
    LVar *lvar;
};

typedef struct Arg Arg;
struct Arg {
    Arg *next;
    char *name;
    int len;
    Type *ty;
};

typedef struct Func Func;
struct Func {
    Func *next;
    Arg *arg;
    LVar *lvar;
    Node *children[100];
    Type *ty;
    char *name;
    int len;
    int depth;
};



extern Token *token; // token sequence
extern char *user_input; // program input
extern LVar *locals; // local_variables;
extern Func *code[100]; // node sequence
extern int id;
extern int toplevel;
extern Tyenv *tyenv;
extern Tyenv *tyenv_fun;
extern int cntptr_ty;

//---------------------------------------------------------------
// Function prototype

// Error function
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
int fresh_id();

// Lexer
bool consume(char *op);
Token *consume_ident();
void expect(char *op);
int expect_number();
LVar *find_lvar(Token *tok);
int is_alnum(char c);
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
void print_tokens(Token *token);
Token *tokenize(char *p);

// Parser
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
char *enum2str(NodeKind kind);
void pprint_node(Node* node, int depth);
void pprint(char *str, int depth);
void print_nodes(Node *node, int depth);
void program();
Func *func();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *term();

// Code generation
void gen_lval(Node *node);
void gen_fun(Func *func);
void gen(Node *node);

// Type Check
void tycheck(Node *node);
void tycheck_fun(Func *func);