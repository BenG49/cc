#pragma once

#include <vector>

#include <lexer.hpp>

struct Func;
struct Node;

struct Symbol {
	TokType type;
	std::string name;

	bool reg;
	int offset_or_reg;

	Node *node;

	Symbol(TokType t, const std::string &name, Node *node, int offset_or_reg, bool reg)
		: type(t), name(name), reg(reg), offset_or_reg(offset_or_reg), node(node) {}
};

// scope deallocates variables on stack after done, stack index is left the same
struct Scope {
	Scope *parent_scope;

	int stack_index;

	std::vector<Symbol> vec;

	Scope(Scope *parent_scope, int stack_index)
		: parent_scope(parent_scope), stack_index(stack_index) {}
	
	bool in_scope(const std::string &name) const
	{
		for (const Symbol &s : vec)
			if (s.name == name)
				return true;

		return false;
	}

	std::pair<int, Scope*> get(const std::string &name) const
	{
		for (unsigned i = 0; i < vec.size(); ++i)
		{
			if (vec[i].name == name)
				return std::pair<int, Scope*>(i, (Scope*)this);
		}

		if (parent_scope)
			return parent_scope->get(name);

		std::cerr << "Undefined identifier " << name << '\n';
		exit(1);
	}
};
