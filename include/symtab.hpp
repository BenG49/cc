#pragma once

#include <vector>

#include <lexer.hpp>

struct Symbol {
    TokType type;
    std::string name;

    Symbol(TokType t, const std::string &name)
        : type(t), name(name) {}
};

struct SymTab
{
    std::vector<Symbol> syms;
};
