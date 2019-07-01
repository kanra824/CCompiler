/*
 * expr = mul ("+" mul | "-" mul)*
 * mul = term ("*" term | "/" term)*
 * term = num | "(" expr ")"
 */

// kind of AST node
typedef enum {
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_NUM, // 整数
} NodeKind;

typedef struct Node Node;

// type of AST Node
struct Node {
    NodeKind kind; // type of node
    Node *lhs; // left child
    Node *rhs; // right child
    int val; // use if kind == ND_NUM
};

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
    Node *node = term();

    for(;;) {
        if(consume('*')) {
            node = new_node(ND_MUL, node, term());
        } else if(consume('/')) {
            node = new_node(ND_DIV, node, term());
        } else {
            return node;
        }
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