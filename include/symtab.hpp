#pragma once

#include <vector>

#include <codegen.hpp>
#include <lexer.hpp>

struct Func;
struct Node;

struct Symbol {
	Size size;
	int val;

	std::string name;
	Node *node;

	bool reg;
	bool global;
	bool pointer;

	Symbol(Size size, const std::string &name, Node *node)
		: size(size), name(name), node(node), reg(false), global(true) {}
	Symbol(Size size, const std::string &name, Node *node, int bp_offset)
		: size(size), val(bp_offset), name(name), node(node), reg(false), global(false) {}
	Symbol(Size size, const std::string &name, Reg r, Node *node)
		: size(size), val(r), name(name), node(node), reg(true), global(false) {}

	
	void emit(Gen &g)
	{
		if (global)
		{
			g.append(name.c_str(), false);
			g.emitc('(');
			g.emit_reg(IP, Quad);
			g.emitc(')');
		}
		else if (reg)
			g.emit_reg(static_cast<Reg>(val), size);
		else
		{
			if (val)
				g.emit_int(val);
			
			g.emitc('(');
			g.emit_reg(BP, Quad);
			g.emitc(')');
		}
	}
};

// scope deallocates variables on stack after done, stack index is left the same
struct Scope {
	Scope *parent_scope;

	int stack_index;
	int size;

	std::vector<Symbol> vec;

	Scope(Scope *parent_scope, int stack_index)
		: parent_scope(parent_scope), stack_index(stack_index), size(0) {}
	
	void add_var(TokType type) {
		Size s = getsize(type);
		int var_size = 1 << s;

		stack_index -= var_size;
		size += var_size;
	}
	
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
