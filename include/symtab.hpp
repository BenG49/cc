#pragma once

#include <vector>

#include <lexer.hpp>

struct Symbol {
	TokType type;
	std::string name;
	int ebp_offset;

	int scope_entry;

	Symbol(TokType t, const std::string &name, int s)
		: type(t), name(name), scope_entry(s) {}
};

struct SymTab {
    std::vector<Symbol> vec;

	int lookup(const std::string &name) const
	{
		for (unsigned i = 0; i < vec.size(); ++i)
		{
			if (vec[i].name == name)
				return i;
		}

		std::cerr << "Undefined variable " << name << '\n';
		exit(1);
	}
};
