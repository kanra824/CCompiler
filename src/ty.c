#include "mdcc.h"

int compare_ty(Type t1, Type t2) {
    Type *ty1;
    ty1->val = t1.val;
    ty1->ty = t1.ty;
    ty1->ptr_to = t1.ptr_to;
    Type *ty2;
    ty2->val = t2.val;
    ty2->ty = t2.ty;
    ty2->ptr_to = t2.ptr_to;
    while(ty1 && ty2) {
        if(ty1->ty == ty2->ty && ty1->val && ty2->val) {
            ty1 = ty1->ptr_to;
            ty2 = ty2->ptr_to;
        } else {
            return 0;
        }
    }
    if(ty1 || ty2) {
        return 0;
    } else {
        return 1;
    }
}

Type tycheck(Node *node) {
    Type ty1, ty2, ty3, ty4;
    Type *tyfun = calloc(1, sizeof(Type));
    int i = 0;
    Tyenv *now = calloc(1, sizeof(Tyenv));
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
                node->ty = t;
                return t;
            } else {
                error("type error at binop");
            }
        case ND_ASSIGN:
            ty2 = tycheck(node->rhs);
            if(ty2.val == RVAL && node->lhs->kind == ND_LVAR) {
                Tyenv *head = calloc(1, sizeof(Tyenv));
                head->str = node->lhs->str;
                head->len = node->lhs->len;
                head->ty = ty2;
                head->ptr_to = tyenv;
                tyenv = head;
                node->ty = ty2;
                return ty2;
            } else if(ty2.val == LVAL && node->lhs->kind == ND_LVAR) {
                Tyenv *head = calloc(1, sizeof(Tyenv));
                head->str = node->lhs->str;
                head->len = node->lhs->len;
                head->ty = *(ty2.ptr_to);
                head->ptr_to = tyenv;
                tyenv = head;
                node->ty = *(ty2.ptr_to);
                return *(ty2.ptr_to);
            } else {
                error("type error at assign");
            }
        case ND_LVAR:
            now = tyenv;
            while(now) {
                if(now->len == node->len && !strncmp(now->str, node->str, now->len)) {
                    node->ty = now->ty;
                    return now->ty;
                } else {
                    now = now->ptr_to;
                }
            }
            error("type error at lvar");
        case ND_RETURN:
            ty1 = tycheck(node->lhs);
            node->ty = ty1;
            return ty1;
        case ND_IF:
            ty1 = tycheck(node->children[0]);
            ty2 = tycheck(node->children[1]);
            ty3 = tycheck(node->children[2]);
            if(ty1.ty == INT) {
                node->ty = ty2;
                return ty2;
            } else {
                error("type error at ifexp");
            }
        case ND_WHILE:
            ty1 = tycheck(node->children[0]);
            ty2 = tycheck(node->children[1]);
            if(ty1.ty == INT) {
                node->ty = ty2;
                return ty2;
            } else {
                error("type error at whileexp");
            }
        case ND_FOR:
            ty1 = tycheck(node->children[0]);
            ty2 = tycheck(node->children[1]);
            ty3 = tycheck(node->children[2]);
            ty4 = tycheck(node->children[3]);
            if(ty2.ty == INT) {
                node->ty = ty4;
                return ty4;
            } else {
                error("type error at forexp");
            }
        case ND_BLOCK:
            while(node->children[i]) {
                ty1 = tycheck(node->children[i]);
                i++;
            }
            return ty1;
        case ND_FUN:
            tyfun->val = RVAL;
            tyfun->ty = FUN;
            while(node->children[i+1]) {
                Tyenv *head = calloc(1, sizeof(Tyenv));
                head->str = node->children[i]->str;
                head->len = node->children[i]->len;
                Type type;
                type.ty = INT;
                head->ty = type; // あとでなおす
                head->ptr_to = tyenv;
                tyenv = head;
                tyfun->param[i] = &type;
                i++;
            }
            ty1 = tycheck(node->children[i]);
            tyfun->ret = &ty1;
            Tyenv *head = calloc(1, sizeof(Tyenv));
            head->ty = ty1;
            head->str = node->str;
            head->len = node->len;
            head->ptr_to = tyenv_fun;
            tyenv_fun = head;
            node->ty = *tyfun;
            return ty1;
        case ND_APP:
            tyfun->val = RVAL;
            tyfun->ty = FUN;
            now = tyenv_fun;
            while(now) {
                if((now->len == node->len) && !strncmp(now->str, node->str, now->len)) {
                    tyfun->val = RVAL;
                    tyfun->ty = FUN;
                    i = 0;
                    while(now->ty.param[i]) {
                        tyfun->param[i] = now->ty.param[i];
                        i++;
                    }
                    tyfun->ret = now->ty.ret;
                    node->ty = *tyfun;
                    return *tyfun;
                } else {
                    now = now->ptr_to;
                }
            }
            error("type error at app");
        case ND_ADDR:
            ty1 = tycheck(node->lhs);
            ty2.val = RVAL;
            ty2.ty = PTR;
            ty2.ptr_to = &ty1;
            node->ty = ty2;
            return ty2;
            break;
        case ND_DEREF:
            ty1 = tycheck(node->lhs);
            node->ty = *ty1.ptr_to;
            return *ty1.ptr_to;
        case ND_NUM:
            ty1.val = RVAL;
            ty1.ty = INT;
            ty1.ptr_to = NULL;
            node->ty = ty1;
            return ty1;
    }           
}
