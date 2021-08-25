#pragma once

#include <vector>

#include <lexer.hpp>

enum ASTType
{
    NODE,
    LEAF,
    LIST,
};

struct AST
{
    ASTType id;

    AST(ASTType id)
        : id(id) {}
};

struct ASTNode : AST
{
    // TODO: add operator
    AST *lhs, *rhs;

    ASTNode(AST *lhs, AST *rhs)
        : AST(NODE), lhs(lhs), rhs(rhs) {}
};

struct ASTLeaf : AST
{
    Token val;

    ASTLeaf(Token val)
        : AST(LEAF), val(val) {}
};

struct ASTList : AST
{
    std::vector<AST *> list;

    ASTList(const std::vector<AST *> list)
        : AST(LIST), list(list) {}
};

// functions

class Parser
{
    Lexer &l;

    // 67 functions oh boy
    AST primary_expr();
    AST postfix_expr();
    AST arg_expr_list();

    void parse_err(const std::string &msg);

public:
    Parser(Lexer &l);

    ASTList parse();
};
