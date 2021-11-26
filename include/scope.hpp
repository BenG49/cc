#pragma once

#include <lexer.hpp>
#include <defs.hpp>

#include <vector>

enum VarType { V_GLOBL, V_VAR, V_REG, V_FUNC };

struct Sym {
	VarType vtype;
	PrimType type;
	std::string name;
	// param count for V_FUNC
	// if assigned or not for V_GLOBL
	// offset if V_VAR
	int val;

	Sym(VarType vtype, PrimType type, const std::string &name)
		: vtype(vtype), type(type), name(name), val(0) {}
	Sym(VarType vtype, PrimType type, const std::string &name, int val)
		: vtype(vtype), type(type), name(name), val(val) {}
};

struct AST;

struct Scope {
	int parent_id;
	int id;

	// total bytes of all vars
	int size;

	std::vector<Sym> syms;

	// entry, id
	AST *get(const std::string &name);
	bool in_scope(const std::string &name);


	// static functions
	static int new_scope(int cur);
	static Scope *s(int id) { return scopes[id]; };

	static const int GLOBAL = 0;

private:
	static std::vector<Scope*> scopes;
	static int scope_count;
};
