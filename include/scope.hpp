#pragma once

#include <lexer.hpp>

#include <vector>

enum VarType { V_GLOBL, V_VAR, V_REG, V_FUNC };

struct Sym {
    VarType vtype;
    TokType type;
    std::string name;
    int val;

    Sym(VarType vtype, TokType type, const std::string &name)
        : vtype(vtype), type(type), name(name) {}
    Sym(VarType vtype, TokType type, const std::string &name, int val)
        : vtype(vtype), type(type), name(name), val(val) {}
};

struct Scope {
    int parent_id;
    int id;

    int size;

    std::vector<Sym> syms;

    std::pair<int, int> get(const std::string &name);
    bool in_scope(const std::string &name);


    // static functions
    static int new_scope(int cur);
    static Scope *s(int id) { return scopes[id]; };

    static const int GLOBAL = 0;

private:
    static std::vector<Scope*> scopes;
    static int scope_count;
};
