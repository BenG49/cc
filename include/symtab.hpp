#pragma once

#include <vector>

#include <codegen.hpp>
#include <lexer.hpp>

struct Func;
struct Node;

struct Symbol {
	Size size;
	std::string name;
	bool reg;
	bool global;
	int reg_or_bp_offset;

	Node *node;

	Symbol(Size size, const std::string &name, Node *node)
		: size(size), name(name), reg(false), global(true), node(node) {}
	Symbol(Size size, const std::string &name, Node *node, int bp_offset)
		: size(size), name(name), reg(false), global(false), reg_or_bp_offset(bp_offset), node(node) {}
	Symbol(Size size, const std::string &name, Reg r, Node *node)
		: size(size), name(name), reg(true), global(false), reg_or_bp_offset(r), node(node) {}

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
			g.emit_reg(static_cast<Reg>(reg_or_bp_offset), size);
		else
		{
			if (reg_or_bp_offset)
				g.emit_int(reg_or_bp_offset);
			
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
