#pragma once

#include <vector>

#include <lexer.hpp>

struct Symbol {
    TokType type;
    const char *name;
};

struct SymTab
{
    std::vector<Symbol> syms;
};
