#pragma once

#include <fstream>
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
    FUNC,
    RET,
    BINOP,
    UNOP,
    COND,
    CONST,
};

struct Node {
    NodeType type;
    virtual void emit(std::ofstream &out) const {}
};
struct Stmt : Node {};
struct Expr : Node {};

struct Block : Stmt {
    std::vector<Stmt *> vec;
    Block() { type = BLOCK; }
    void emit(std::ofstream &out) const override;
};

/*struct If : Stmt {
    Expr *cond;
    Stmt *if_blk, *else_blk;
    If() { type = IF; }
};

struct For : Stmt {
    Stmt *init, *inc, *blk;
    Expr *cond;
    For() { type = FOR; }
};

struct While : Stmt {
    Stmt *blk;
    Expr *cond;
    While() { type = WHILE; }
};

struct Decl : Stmt {
    TokType type;
    Symbol sym;
    Expr *expr;
    Decl() { type = DECL; }
};

struct Assign : Stmt {
    TokType op;
    Expr *lval, rval;
    Assign() { type = ASSIGN; }
};*/

struct Func : Stmt {
    // [0] = function symbol
    std::vector<Symbol> name_params;
    Stmt *blk;
    Func() { type = FUNC; }
    void emit(std::ofstream &out) const override;
};

struct Ret : Stmt {
    Expr *r;
    Ret() { type = RET; }
    void emit(std::ofstream &out) const override;
};

//

/*struct BinOp : Expr {
    TokType op;
    Expr *lhs, *rhs;
    BinOp() { type = BINOP; }
};*/

struct UnOp : Expr {
    TokType op;
    Expr *operand;
    UnOp() { type = UNOP; }
    void emit(std::ofstream &out) const override;
};

/*struct Cond : Expr {
    Expr *cond, t, f;
    Cond() { type = COND; }
};*/

struct Const : Expr {
    Token t;
    Const(const Token &t) : t(t) { type = CONST; }
    void emit(std::ofstream &out) const override;
};

// 

class Parser
{
    Lexer &l;

    Expr *expr();
    Stmt *statement();
    Ret *returnstatement();
    Func *function();
    Block *block();
    UnOp *unop();
    // Node *ifstatement();
    // Node *decl();
    // TokType type();

    bool is_type(TokType t);

public:
    Parser(Lexer &l)
        : l(l) {}

    Block *parse();
};
