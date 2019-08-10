#include "mdcc.h"

Type tycheck(Node *node) {
    Type ty1, ty2, ty3, ty4;
    int i = 0;
    Tyenv *head;
    switch(node->kind) {
        case ND_ADD:
        case ND_SUB:
        case ND_MUL:
        case ND_DIV:
        case ND_BEQ:
        case ND_NEQ:
        case ND_LT:
        case ND_LE:
            ty1 = tycheck(node->lhs);
            ty2 = tycheck(node->rhs);
            if(ty1.ty == INT && ty2.ty == INT) {
                Type t = {RVAL, INT, NULL};
                return t;
            } else {
                error("type error at binop");
            }
            break;
        case ND_ASSIGN:
            ty2 = tycheck(node->rhs);
            if(ty2.val == RVAL && node->lhs->kind == ND_LVAR) {
                head->offset = node->lhs->offset;
                head->ty = ty2;
                head->ptr_to = tyenv;
                tyenv = head;
                return ty2;
            } else if(ty2.val == LVAL && node->lhs->kind == ND_LVAR) {
                head->offset = node->lhs->offset;
                head->ty = *ty2.ptr_to;
                head->ptr_to = tyenv;
                tyenv = head;
                return ty2;
            } else {
                error("type error at assign");
            }
            break;
        case ND_LVAR:
            break;
        case ND_RETURN:
            ty1 = tycheck(node->lhs);
            return ty1;
            break;
        case ND_IF:
            ty1 = tycheck(node->children[0]);
            ty2 = tycheck(node->children[1]);
            ty3 = tycheck(node->children[2]);
            if(ty1.ty == INT) {
                return ty1;
            } else {
                error("type error at ifexp");
            }
            break;
        case ND_WHILE:
            ty1 = tycheck(node->children[0]);
            ty2 = tycheck(node->children[1]);
            if(ty1.ty == INT) {
                return ty2;
            } else {
                error("type error at whileexp");
            }
            break;
        case ND_FOR:
            ty1 = tycheck(node->children[0]);
            ty2 = tycheck(node->children[1]);
            ty3 = tycheck(node->children[2]);
            ty4 = tycheck(node->children[3]);
            if(ty1.ty == INT && ty2.ty == INT) {
                return ty4;
            } else {
                error("type error at whileexp");
            }
            break;
        case ND_BLOCK:
            while(node->children[i]) {
                ty1 = tycheck(node->children[i]);
                i++;
            }
            return ty1;
            break;
        case ND_FUN:
            break;
        case ND_APP:
            break;
        case ND_ADDR:
            ty1 = tycheck(node->lhs);
            ty2.val = RVAL;
            ty2.ty = PTR;
            ty2.ptr_to = &ty1;
            return ty2;
            break;
        case ND_DEREF:
            ty1 = tycheck(node->lhs);
            return *ty1.ptr_to;
            break;
        case ND_NUM:
            ty1.val = RVAL;
            ty1.ty = INT;
            ty1.ptr_to = NULL;
            return ty1;
            break;
    }           
}
