#pragma once

#include <fstream>
#include <vector>

#include <lexer.hpp>
#include <scope.hpp>
#include <defs.hpp>

struct AST {
	NodeType type;
	PrimType ptype;
	AST *lhs, *mid, *rhs;

	int val;
	int scope_id;

	Sym &get_sym() const { return Scope::s(scope_id)->syms[val]; }

	// appends to an "ast list"
	// ordering: top down, left to right
	static AST *append(AST *bottom, AST *node, NodeType t)
	{
		// if lhs available
		if (!bottom->lhs)
		{
			bottom->lhs = node;
			return bottom;
		}
		// if mid available
		else if (!bottom->mid)
		{
			bottom->mid = node;
			return bottom;
		}
		// none are avaiable, add ast to bottom of tree
		else
		{
			AST *new_bottom = new AST(t, INT, node);
			bottom->rhs = new_bottom;
			return new_bottom;
		}
	}

	// generic constructors
	AST(NodeType type, PrimType p, AST *lhs, AST *mid, AST *rhs)
		: type(type), ptype(p), lhs(lhs), mid(mid), rhs(rhs), val(0) {}

	AST(NodeType type, PrimType p, AST *lhs, AST *rhs)
		: AST(type, p, lhs, nullptr, rhs) {}

	AST(NodeType type, PrimType p, AST *lhs)
		: AST(type, p, lhs, nullptr, nullptr) {}

	AST(NodeType type)
		: AST(type, INT, nullptr, nullptr, nullptr) {}

	// val leaf
	AST(NodeType type, PrimType p, int val)
		: type(type), ptype(p), lhs(nullptr), mid(nullptr), rhs(nullptr), val(val) {}

	// var
	AST(PrimType p, int entry, int scope_id)
		: type(VAR), ptype(p), lhs(nullptr), mid(nullptr), rhs(nullptr), val(entry), scope_id(scope_id) {}

	// to be safe
	AST()
		: lhs(nullptr), mid(nullptr), rhs(nullptr), val(0) {}
};

class ASTIter {
	const AST *cur;
	int n;

public:
	ASTIter(const AST *list)
		: cur(list), n(0) {}
	
	bool has_next()
	{
		if (cur)
		{
			switch (n) {
				case 0: return cur->lhs;
				case 1: return cur->mid;
				case 2: return cur->rhs && cur->rhs->lhs;
				default: return false;
			}
		}
		else
			return false;
	}
	
	AST *next()
	{
		switch (n++) {
			case 0: return cur->lhs;
			case 1: return cur->mid;
			case 2:
				cur = cur->rhs;
				n = 0;
				return next();
			default: return nullptr;
		}
	}
};

// -------- class def -------- //

class Parser
{
	Lexer &l;

	// context
	int cur_scope;
	int offset;
	int stk_size;
	bool in_loop;

	AST *expr();
	AST *exp_option();
	AST *stmt();

	AST *func();
	AST *decl();
	AST *if_stmt();
	AST *for_stmt();
	AST *while_stmt();
	AST *do_stmt();

	AST *compound(bool newscope=true);

	AST *lval();
	AST *assign();

	AST *cond();
	AST *op_or();
	AST *op_and();
	AST *bitwise_or();
	AST *bitwise_xor();
	AST *bitwise_and();
	AST *equality();
	AST *comparison();
	AST *shift();
	AST *term();
	AST *factor();
	AST *unop();
	AST *postfix();
	AST *primary();
	AST *call();

public:
	Parser(Lexer &l)
		: l(l)
		// create global scope
		, cur_scope(Scope::new_scope(0))
		, offset(0)
		, stk_size(0)
		, in_loop(false) {}

	AST *parse();

	static NodeType asnode(TokType t);
	static PrimType asptype(TokType t);
};
