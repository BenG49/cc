#pragma once

#include <vector>

#include <symtab.hpp>
#include <lexer.hpp>

enum NodeType
{
    DEFAULT,
    LIST,
    TOKTYPE,
    IF,
    BIN_OP,
    FUNC,
    PARAMS,
    VAR,
    VARLIST,
    TOKEN,
};

struct Node;

struct DfltNode {
    Node *lhs, *rhs;
};

struct ListNode {
    std::vector<Node *> vec;
};

struct IfNode {
    Node *cond;

    std::vector<Node *> bodies;
};

struct BinOp {
    char op;
    Node *lhs, *rhs;
};

struct FuncNode {
    Symbol *name;
    Node *params, *body;
};

struct VarNode {
    Symbol *s;
};

struct VarList {
    std::vector<Symbol *> vec;
};

struct TokNode {
    Token t;
};

// 

struct Node
{
    NodeType type;

    union {
        DfltNode node; 
        ListNode listnode;
        TokType t;
        IfNode ifnode;
        BinOp binnode;
        FuncNode func;
        VarNode var;
        VarList vlist;
        TokNode tok;
    };

    Node() {}
};

class Parser
{
    Lexer &l;

    Node *new_node(NodeType type);

    Node *expr();
    Node *statement();
    Node *returnstatement();
    // Node *block();
    // Node *ifstatement();
    // Node *function();
    // Node *decl();
    // TokType type();

public:
    Parser(Lexer &l)
        : l(l) {}

    Node *parse();
};
