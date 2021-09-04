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
	POSTFIX,
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
	std::vector<Symbol> *vars;
	Var() { type = VAR; }
	Var(int entry, Scope *scope)
		: entry(entry)
		, vars(&scope->vec) { type = VAR; }
	void emit(Gen &g) const override;
};

struct Compound : Stmt {
	std::vector<Node *> vec;
	Scope *scope;
	bool func;
	Compound() : func(false) { type = BLOCK; }
	Compound(Scope *scope) : scope(scope), func(false) { type = BLOCK; }
	void emit(Gen &g) const override;
};

struct If : Stmt {
	Expr *cond;
	Node *if_blk, *else_blk;
	If() { type = IF; }
	If(Expr *cond, Node *if_blk)
		: cond(cond)
		, if_blk(if_blk) { type = IF; }
	If(Expr *cond, Node *if_blk, Node *else_blk)
		: cond(cond)
		, if_blk(if_blk)
		, else_blk(else_blk) { type = IF; }
	void emit(Gen &g) const override;
};

/*struct For : Stmt {
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
	Compound *blk;
	Func() { type = FUNC; }
	void emit(Gen &g) const override;
};

struct Ret : Stmt {
	Expr *r;
	Ret() { type = RET; }
	Ret(Expr *r) : r(r) { type = RET; }
	void emit(Gen &g) const override;
};

struct Decl : Stmt {
	Var v;
	Expr *expr;
	Decl(Var v)
		: v(v)
		, expr(nullptr) { type = DECL; }
	Decl(Var v, Expr *expr)
		: v(v)
		, expr(expr) { type = DECL; }
	void emit(Gen &g) const override;
};

// -------- ast expressions -------- //

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

struct Post : Expr {
	TokType op;
	Expr *operand;
	Post() { type = POSTFIX; }
	Post(TokType op, Expr *operand)
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

struct Cond : Expr {
	Expr *cond, *t, *f;
	Cond() { type = COND; }
	Cond(Expr *cond, Expr *t, Expr *f)
		: cond(cond)
		, t(t)
		, f(f) { type = COND; }
	void emit(Gen &g) const override;
};

struct Const : Expr {
	Token t;
	Const(const Token &t) : t(t) { type = CONST; }
	void emit(Gen &g) const override;
};

// -------- class def -------- //

class Parser
{
	Lexer &l;

	Scope *scope;

	Expr *expr();
	Node *stmt();

	Stmt *func();
	Stmt *decl();
	Stmt *if_stmt();

	Compound *compound(bool newscope=true);

	Expr *lval();
	Expr *assign();

	Expr *cond();
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
	Expr *postfix();
	Expr *primary();

	void parse_err(const std::string &msg, const Token &err_tok);

public:
	Parser(Lexer &l)
		: l(l)
		, scope(new Scope(nullptr, 0)) {}

	Compound *parse();
};
