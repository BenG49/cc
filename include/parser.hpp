#pragma once

#include <fstream>
#include <vector>

#include <codegen.hpp>
#include <symtab.hpp>
#include <lexer.hpp>

enum NodeType
{
	BLOCK,
	IF,
	FOR,
	WHILE,
	DECL,
	ASSIGN,
	FUNC,
	RET,
	BINOP,
	UNOP,
	COND,
	CONST,
	VAR,
};

struct Node {
	NodeType type;
	virtual void emit(Gen &g) const {}
};
struct Stmt : Node {};
struct Expr : Node {};

struct Var : Expr {
	int entry;
	Var() { type = VAR; }
	Var(int entry) : entry(entry) { type = VAR; }
	void emit(Gen &g) const override;
};

struct Block : Stmt {
	std::vector<Node *> vec;
	Block() { type = BLOCK; }
	void emit(Gen &g) const override;
};

/*struct If : Stmt {
	Expr *cond;
	Stmt *if_blk, *else_blk;
	If() { type = IF; }
};

struct For : Stmt {
	Stmt *init, *inc, *blk;
	Expr *cond;
	For() { type = FOR; }
};

struct While : Stmt {
	Stmt *blk;
	Expr *cond;
	While() { type = WHILE; }
};
*/

struct Func : Stmt {
	Var name;
	std::vector<Symbol> params;
	Stmt *blk;
	int offset;
	Func() : offset(0) { type = FUNC; }
	void emit(Gen &g) const override;
};

struct Ret : Stmt {
	Expr *r;
	Ret() { type = RET; }
	void emit(Gen &g) const override;
};

struct Decl : Stmt {
	Var v;
	Expr *expr;
	Decl(Var v) : v(v) { type = DECL; }
	Decl(Var v, Expr *expr)
		: v(v)
		, expr(expr) { type = DECL; }
	void emit(Gen &g) const override;
};

//

struct BinOp : Expr {
	TokType op;
	Expr *lhs, *rhs;
	BinOp() { type = BINOP; }
	BinOp(TokType op, Expr *lhs, Expr *rhs)
		: op(op)
		, lhs(lhs)
		, rhs(rhs) { type = BINOP; }
	void emit(Gen &g) const override;
};

struct UnOp : Expr {
	TokType op;
	Expr *operand;
	UnOp() { type = UNOP; }
	UnOp(TokType op, Expr *operand)
		: op(op)
		, operand(operand) { type = UNOP; }
	void emit(Gen &g) const override;
};

struct Assign : Expr {
	TokType op;
	Expr *lval, *rval;
	Assign() { type = ASSIGN; }
	Assign(TokType op, Expr *lval, Expr *rval)
		: op(op)
		, lval(lval)
		, rval(rval) { type = ASSIGN; }
	void emit(Gen &g) const override;
};

struct Const : Expr {
	Token t;
	Const(const Token &t) : t(t) { type = CONST; }
	void emit(Gen &g) const override;
};

// 

class Parser
{
	Lexer &l;
	SymTab &s;

	Func *cur_func;

	Expr *expr();
	Node *statement();

	Stmt *returnstatement();
	Stmt *function();
	Stmt *block();
	Stmt *decl();

	Expr *assign();

	Expr *binop();
	Expr *op_or();
	Expr *op_and();
	Expr *bitwise_or();
	Expr *bitwise_xor();
	Expr *bitwise_and();
	Expr *equality();
	Expr *comparison();
	Expr *shift();
	Expr *term();
	Expr *factor();
	Expr *unop();
	Expr *primary();

public:
	Parser(Lexer &l, SymTab &s)
		: l(l), s(s) {}

	Block *parse();
};
