#pragma once

#include <vector>

#include <lexer.hpp>

enum ASTType
{
    NORMAL,
    LIST
};

struct AST
{
    ASTType id;
};

struct ASTNode : AST
{
    AST *lhs, *rhs;
};

struct ASTList : AST
{
    std::vector<AST *> list;
};

// functions

class Parser
{
    Lexer &l;

    AST parse_stmt();
    AST parse_func();
    AST parse_decl();

public:
    ASTList parse();
};
