#pragma once

#include <fstream>
#include <vector>

#include <lexer.hpp>
#include <scope.hpp>

enum NodeType {
	NONE,
	LIST,
	IF,
	FOR,
	FOR_DECL,
	WHILE,
	DO,
	DECL,
	ASSIGN,
	FUNC,
	RET,
	BINOP,
	UNOP,
	POSTFIX,
	COND,
	CONST,
	VAR,
	BREAK,
	CONT,
	CALL,
	PTR,
};

struct AST {
	NodeType type;
	TokType op;
	AST *lhs, *mid, *rhs;

	int val;
	int scope_id;

	Sym &get_sym() { return Scope::s(scope_id)->syms[val]; }

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
			AST *new_bottom = new AST(t, node);
			bottom->rhs = new_bottom;
			return new_bottom;
		}
	}

	// generic constructors
	AST(NodeType type, AST *lhs, AST *mid, AST *rhs)
		: type(type), lhs(lhs), mid(mid), rhs(rhs), val(0) {}

	AST(NodeType type, AST *lhs, AST *rhs)
		: type(type), lhs(lhs), mid(nullptr), rhs(rhs) {}

	AST(NodeType type, AST *lhs)
		: AST(type, lhs, nullptr, nullptr) {}

	AST(NodeType type)
		: AST(type, nullptr, nullptr, nullptr) {}

	// op constructors
	AST(NodeType type, TokType op, AST *lhs, AST *rhs)
		: type(type), op(op), lhs(lhs), mid(nullptr), rhs(rhs) {}

	AST(NodeType type, TokType op, AST *lhs)
		: AST(type, op, lhs, nullptr) {}

	// val leaf
	AST(NodeType type, int val)
		: type(type), lhs(nullptr), mid(nullptr), rhs(nullptr), val(val) {}

	// var
	AST(int entry, int scope_id)
		: type(VAR), lhs(nullptr), mid(nullptr), rhs(nullptr), val(entry), scope_id(scope_id) {}

	// to be safe
	AST()
		: lhs(nullptr), mid(nullptr), rhs(nullptr) {}
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

	void parse_err(const std::string &msg, const Token &err_tok);

public:
	Parser(Lexer &l)
		: l(l)
		// create global scope
		, cur_scope(Scope::new_scope(0))
		, offset(0)
		, stk_size(0)
		, in_loop(false) {}

	AST *parse();
};
