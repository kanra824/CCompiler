#include "mdcc.h"

// Lexer
/* if next token is expected to be a symbol
 * then read next token and return true
 * else return false
 */
bool consume(char *op) {
/*     printf("token->str: %s\n", token->str);
    printf("op: %s\n", op);
    printf("token->len: %d\n", token->len);
    printf("strlen(op): %ld\n", strlen(op)); */
    if(token->kind != TK_RESERVED ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len)) {
            return false;
    }
    token = token->next;
    return true;
}

Token *consume_ident() {
    if(token->kind == TK_IDENT) {
        Token *tok = calloc(1, sizeof(Token));
        tok->kind = TK_IDENT;
        tok->str = token->str;
        tok->len = 1;
        return tok;
    } else {
        return NULL;
    }
}

/* if next token is expected symbol
 * then read next token
 * else report error
 */
void expect(char *op) {
    if(token->kind != TK_RESERVED ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len)) {
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
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    if(len != -1) {
        tok->str = (char*)malloc(sizeof(char) * len);
        if(tok->str == NULL) {
            error("tokstr can't reserved");
        }
        strncpy(tok->str, str, len);
    }
    tok->len = len;
    cur->next = tok;
    return tok;
}

void print_tokens(Token *token) {
    if(!(token == NULL)) {
        switch(token->kind) {
            case TK_EOF:
                printf("TK_EOF\n");
                return;
                break;
            case TK_NUM:
                printf("TK_NUM: %d\n", token->val);
                break;
            default:
                printf("TK_RESERVED: %s\n", token->str);
                break;
        }
    }
    print_tokens(token->next);
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
        } else if(!strncmp(p, ">=", 2) || !strncmp(p, "<=", 2) ||
            !strncmp(p, "==", 2) || !strncmp(p, "!=", 2)) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
        } else if(*p == '+' || *p == '-' || *p == '*' || *p == '/' ||
           *p == '(' || *p == ')' || *p == '>' || *p == '<' ||
           *p == '=' || *p == ',' || *p == ';') {
            cur = new_token(TK_RESERVED, cur, p++, 1);
        } else if('a' <= *p && *p <= 'z') {
            cur = new_token(TK_IDENT, cur, p++, 1);
        } else if(isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, -1);
            cur->val = strtol(p, &p, 10);
        } else {
            error_at(p, "Cant tokenize");
        }
    }

    new_token(TK_EOF, cur, p, -1);
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

void pprint_node(char *str, int val, int depth) {
    for(int i=0;i<depth;++i) {
        printf("\t");
    }
    printf("%s", str);
    if(val == -1) {
        printf("\n");
    } else {
        printf(": %d\n", val);
    }
}

void print_nodes(Node *node, int depth) {
    if(node == NULL) return;
    switch(node->kind) {
        case ND_ADD:
            print_nodes(node->lhs, depth + 1);
            pprint_node("ND_ADD", -1, depth);
            print_nodes(node->rhs, depth + 1);
            break;
        case ND_SUB:
            print_nodes(node->lhs, depth + 1);
            pprint_node("ND_SUB", -1, depth);
            print_nodes(node->rhs, depth + 1);
            break;
        case ND_MUL:
            print_nodes(node->lhs, depth + 1);
            pprint_node("ND_MUL", -1, depth);
            print_nodes(node->rhs, depth + 1);
            break;
        case ND_DIV:
            print_nodes(node->lhs, depth + 1);
            pprint_node("ND_DIV", -1, depth);
            print_nodes(node->rhs, depth + 1);
            break;
        default:
            pprint_node("ND_NUM", node->val, depth);
            break;
    }
}

Node *code[100];

void program() {
    int i = 0;
    while(!at_eof()) {
        code[++i] = stmt();
    }
    code[i] = NULL;
}

Node *stmt() {
    Node *node = expr();
    expect(";");
    return node;
}

Node *expr() {
    return assign();
}

Node *assign() {
    Node *node = equality();
    if(consume("=")) {
        node = new_node(ND_ASSIGN, node, assign());
    }
    return node;
}

Node *equality() {
    Node *node = relational();
    while(1) {
        if(consume("==")) {
            node = new_node(ND_BEQ, node, relational());
        } else if(consume("!=")) {
            node = new_node(ND_NEQ, node, relational());
        } else {
            return node;
        }
    }
}

Node *relational() {
    Node *node = add();
    while(1) {
        if(consume("<")) {
            node = new_node(ND_LT, node, add());
        } else if(consume("<=")) {
            node = new_node(ND_LE, node, add());
        } else if(consume(">")) {
            node = new_node(ND_LT, add(), node);
        } else if(consume(">=")) {
            node = new_node(ND_LE, add(), node);
        } else {
            return node;
        }
    }
}

Node *add() {
    Node *node = mul();
    while(1) {
        if(consume("+")) {
            node = new_node(ND_ADD, node, mul());
        } else if(consume("-")) {
            node = new_node(ND_SUB, node, mul());
        } else {
            return node;
        }
    }
}

Node *mul() {
    Node *node = unary();
    for(;;) {
        // printf("mul: %s\n", token->str);
        if(consume("*")) {
            node = new_node(ND_MUL, node, unary());
        } else if(consume("/")) {
            node = new_node(ND_DIV, node, unary());
        } else {
            return node;
        }
    }
}

Node *unary() {
    // printf("unary: %s\n", token->str);
    if(consume("+")) {
        return term();
    } else if(consume("-")) {
        return new_node(ND_SUB, new_node_num(0), term());
    } else {
        return term();
    }
}

Node *term() {
    // if next_token == '(' then term must be '(' expr ')'
    // printf("term: %s\n", token->str);
    Token *tok = consume_ident();
    if(tok) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;
        node->offset = (tok->str[0] - 'a' + 1) * 8;
        return node;
    } else if(consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    } else {
        // term muse be a number
        return new_node_num(expect_number());
    }
}