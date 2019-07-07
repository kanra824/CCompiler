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
    if((token->kind != TK_RESERVED) ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len)) {
            return false;
    }
    token = token->next;
    return true;
}

Token *consume_ident() {
    if(token != NULL && token->kind == TK_IDENT) {
        Token *tok = calloc(1, sizeof(Token));
        tok->kind = TK_IDENT;
        tok->str = token->str;
        tok->len = token->len;
        token = token->next;
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
    // TODO: error_atがおかしい！！！！！
    // printf("%s\n", op);
    if(token->kind != TK_RESERVED ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len)) {
            error_at(token->str, "It's not %s\n", op);
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

LVar *find_lvar(Token *tok) {
    for(LVar *var = locals; var; var = var->next) {
        if(var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
            return var;
        }
    }
    return NULL;
}

int is_alnum(char c) {
    return ('a' <= c && c <= 'z') ||
            ('A' <= c && c <= 'Z') ||
            ('0' <= c && c <= '9') ||
            (c == '_');
}

bool at_eof() {
    return token->kind == TK_EOF;
}

// create new token and connect to cur
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    // printf("%s\n", str);
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

void print_tokens(Token *token) {
    if(token != NULL) {
        switch(token->kind) {
            case TK_EOF:
                printf("TK_EOF\n");
                return;
                break;
            case TK_NUM:
                printf("TK_NUM: %d\n", token->val);
                break;
            case TK_IDENT:
                printf("TK_IDENT: %s\n", token->str);
                break;
            case TK_RESERVED:
                printf("TK_RESERVED: %.*s\n", token->len,  token->str);
                break;
            default:
                error("Can't find token in print_tokens");
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
        // printf("%ld\n", ph - p);
        // printf("%ld\n", cur - &head);
        while(*p && *p != ';') {
            // skip space
            if(isspace(*p)) {
                p++;
            } else if(!strncmp(p, "return", 6) && !is_alnum(p[6])) {
                cur = new_token(TK_RESERVED, cur, p, 6);
                p += 6;
            } else if(!strncmp(p, "while", 5) && !is_alnum(p[5])) {
                cur = new_token(TK_RESERVED, cur, p, 5);
                p += 5;
            } else if(!strncmp(p, "else", 4) && !is_alnum(p[4])) {
                cur = new_token(TK_RESERVED, cur, p, 4);
                p += 4;
            } else if(!strncmp(p, "for", 3) && !is_alnum(p[3])) {
                cur = new_token(TK_RESERVED, cur, p, 3);
                p += 3;
            } else if(!strncmp(p, "if", 2) && !is_alnum(p[2])) {
                cur = new_token(TK_RESERVED, cur, p, 2);
                p += 2;
            } else if(!strncmp(p, ">=", 2) || !strncmp(p, "<=", 2) ||
                        !strncmp(p, "==", 2) || !strncmp(p, "!=", 2)) {
                cur = new_token(TK_RESERVED, cur, p, 2);
                p += 2;
            } else if(*p == '+' || *p == '-' || *p == '*' || *p == '/' ||
                        *p == '(' || *p == ')' || *p == '>' || *p == '<' ||
                        *p == '=' || *p == ',') {
                cur = new_token(TK_RESERVED, cur, p++, 1);
            } else if('a' <= *p && *p <= 'z') {
                char *head = p;
                while(('a' <= *p && *p <= 'z') || ('0' <= *p && *p <= '9')) {
                    p++;
                }
                int len = p - head;
                cur = new_token(TK_IDENT, cur, head, len);
                p = head + len;
            } else if(isdigit(*p)) {
                cur = new_token(TK_NUM, cur, p, -1);
                cur->val = strtol(p, &p, 10);
            } else {
                error_at(p, "Cant tokenize");
            }
        }
        if(*p == ';') {
            cur = new_token(TK_RESERVED, cur, p++, 1);
        } else {
            error_at(p, "End of sentence must be a semicolon");
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

char *enum2str(NodeKind kind) {
    switch(kind) {
        case ND_ADD:
            return "+";
        case ND_SUB:
            return "-";
        case ND_MUL:
            return "*";
        case ND_DIV:
            return "/";
        case ND_BEQ:
            return "==";
        case ND_NEQ:
            return "!=";
        case ND_LT:
            return "<";
        case ND_LE:
            return "<=";
        case ND_ASSIGN:
            return "=";
        case ND_RETURN:
            return "ret";
        case ND_IF:
            return "if";
        case ND_WHILE:
            return "while";
        case ND_FOR:
            return "for";
        default:
            return "";
    }
}

void pprint_node(NodeKind kind, int val, int depth) {
    for(int i=0;i<depth;++i) {
        printf("  ");
    }
    printf("%s", enum2str(kind));
    if(val == -1) {
        printf("\n");
    } else if(kind == ND_LVAR) {
        printf("%c\n", 'a' + (val / 8) - 1);
    } else {
        printf("%d\n", val);
    }
}

void pprint(char *str, int depth) {
    for(int i=0;i<depth;++i) {
        printf("  ");
    }
    printf("%s\n", str);
}

void print_nodes(Node *node, int depth) {
    if(node == NULL) return;
    switch(node->kind) {
        case ND_NUM:
            pprint_node(ND_NUM, node->val, depth);
            break;
        case ND_LVAR:
            pprint_node(ND_LVAR, node->offset, depth);
            break;
        case ND_IF:
            pprint_node(ND_IF, -1, depth);
            print_nodes(node->children[0], depth + 1);
            pprint("then", depth);
            print_nodes(node->children[1], depth + 1);
            pprint("else", depth);
            print_nodes(node->children[2], depth + 1);
            pprint("endif", depth);
            break;
        case ND_WHILE:
            pprint_node(ND_WHILE, -1, depth);
            print_nodes(node->children[0], depth + 1);
            pprint("do", depth);
            print_nodes(node->children[1], depth + 1);
            pprint("endwhile", depth);
            break;
        case ND_FOR:
            pprint_node(ND_FOR, -1, depth);
            print_nodes(node->children[0], depth + 1);
            print_nodes(node->children[1], depth + 1);
            print_nodes(node->children[2], depth + 1);
            pprint("do", depth);
            print_nodes(node->children[3], depth + 1);
            pprint("endfor", depth);
            break;
        default:
            print_nodes(node->lhs, depth + 1);
            pprint_node(node->kind, -1, depth);
            print_nodes(node->rhs, depth + 1);
    }
}

void program() {
    int i = 0;
    while(!at_eof()) {
        code[i++] = stmt();
    }
    
    code[i] = NULL;
}

Node *stmt() {
    Node *node;
    if(consume("if")) {
        node = new_node(ND_IF, NULL, NULL);        
        expect("(");
        node->children[0] = expr();
        expect(")");
        node->children[1] = stmt();
        if(consume("else")) {
            node->children[2] = stmt();
        } else {
            node->children[2] = NULL;
        }
        return node;
    } else if(consume("while")) {
        node = new_node(ND_WHILE, NULL, NULL);
        expect("(");
        node->children[0] = expr();
        expect(")");
        node->children[1] = stmt();
        return node;
    } else if(consume("for")) {
        node = new_node(ND_FOR, NULL, NULL);
        expect("(");
        if(!consume(";")) {
            node->children[0] = expr();
            expect(";");
        }
        if(!consume(";")) {
            node->children[1] = expr();
            expect(";");
        }
        if(!consume(")")) {
            node->children[2] = expr();
            expect(")");
        }
        node->children[3] = stmt();
        return node;
    } else if(consume("return")) {
        node = new_node(ND_RETURN, expr(), NULL);
        expect(";");
        return node;
    } else {
        node = expr();
        expect(";");
        return node;
    }
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
        LVar *lvar = find_lvar(tok);
        if(lvar) {
            node->offset = lvar->offset;
        } else {
            lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;
            lvar->name = tok->str;
            lvar->len = tok->len;
            lvar->offset = locals->offset + 8;
            node->offset = lvar->offset;
            locals = lvar;
        }
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