#pragma once

#include <vector>

#include <symtab.hpp>
#include <lexer.hpp>

enum NodeType
{
    BLOCK,
    IF,
    FOR,
    WHILE,
    DECL,
    ASSIGN,
    RET,
    BINOP,
    UNOP,
    COND,
    LEAF,
};

struct Node {
    NodeType type;
};
struct Stmt : Node {};
struct Expr : Node {};

struct Block : Stmt {
    std::vector<Stmt *> vec;
};

struct If : Stmt {
    Expr *cond;
    Stmt *if_blk, *else_blk;
};

struct For : Stmt {
    Stmt *init, *inc, *blk;
    Expr *cond;
};

struct While : Stmt {
    Stmt *blk;
    Expr *cond;
};

struct Decl : Stmt {
    TokType type;
    Symbol *sym;
    Expr *expr;
};

struct Assign : Stmt {
    TokType op;
    Expr *lval, rval;
};

struct Ret : Stmt {
    Expr *r;
};

//

struct BinOp : Expr {
    TokType op;
    Expr *lhs, *rhs;
};

struct UnOp : Expr {
    TokType op;
    Expr *operand;
};

struct Cond : Expr {
    Expr *cond, t, f;
};

struct Leaf : Expr {
    Token t;
    Leaf(const Token &t) : t(t) {}
};

// 

class Parser
{
    Lexer &l;

    Expr *expr();
    Stmt *statement();
    Ret *returnstatement();
    // Node *function();
    // Node *block();
    // Node *ifstatement();
    // Node *decl();
    // TokType type();

public:
    Parser(Lexer &l)
        : l(l) {}

    Block *parse();
};
