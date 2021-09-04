#pragma once

#include <vector>

#include <lexer.hpp>

struct Func;

struct Symbol {
	TokType type;
	std::string name;
	int bp_offset;

	Symbol(TokType t, const std::string &name)
		: type(t), name(name) {}
};

struct SymTab {
	SymTab *parent_scope;

	// this is probably bad and wont work for globals but whatever
	int *bp_offset;

	std::vector<Symbol> vec;

	SymTab(SymTab *parent_scope, int *bp_offset)
		: parent_scope(parent_scope), bp_offset(bp_offset) {}

	std::pair<int, SymTab*> get(const std::string &name) const
	{
		for (unsigned i = 0; i < vec.size(); ++i)
		{
			if (vec[i].name == name)
				return std::pair<int, SymTab*>(i, (SymTab*)this);
		}

		if (parent_scope)
			return parent_scope->get(name);

		std::cerr << "Undefined variable " << name << '\n';
		exit(1);
	}
};
