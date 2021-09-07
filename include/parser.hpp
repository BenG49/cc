#pragma once

#include <fstream>
#include <vector>

#include <codegen.hpp>
#include <symtab.hpp>
#include <lexer.hpp>

enum NodeType {
	NONE,
	BLOCK,
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

	// NOTE: should only be called after parsing is done and vec size will not change
	Symbol &get() const { return (*vars)[entry]; }

	void mov(bool from_var, Gen &g) const;
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
		, if_blk(if_blk)
		, else_blk(nullptr) { type = IF; }
	If(Expr *cond, Node *if_blk, Node *else_blk)
		: cond(cond)
		, if_blk(if_blk)
		, else_blk(else_blk) { type = IF; }
	void emit(Gen &g) const override;
};

struct For : Stmt {
	Expr *init, *cond;
	Node *blk, *post;
	For() { type = FOR; }
	void emit(Gen &g) const override;
};

struct Decl;

struct ForDecl : Stmt {
	Scope *for_scope;
	Decl *init;
	Expr *cond;
	Node *blk, *post;
	ForDecl() { type = FOR_DECL; }
	void emit(Gen &g) const override;
};

struct While : Stmt {
	Node *blk;
	Expr *cond;
	While() { type = WHILE; }
	void emit(Gen &g) const override;
};

struct Do : Stmt {
	Node *blk;
	Expr *cond;
	Do() { type = DO; }
	void emit(Gen &g) const override;
};

struct Func : Stmt {
	Var name;
	std::vector<Var> params;
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
	Decl() { type = DECL; }
	Decl(Var v)
		: v(v)
		, expr(nullptr) { type = DECL; }
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

struct Call : Expr {
	Var func;
	std::vector<Expr*> params;
	Call() { type = CALL; }
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

	// context
	Scope *scope;
	bool in_loop;

	Expr *expr();
	Expr *exp_option();
	Node *stmt();

	Stmt *func();
	Decl *decl();
	Stmt *if_stmt();
	Stmt *for_stmt();
	Stmt *while_stmt();
	Stmt *do_stmt();

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
	Expr *call();

	void parse_err(const std::string &msg, const Token &err_tok);

public:
	Parser(Lexer &l)
		: l(l)
		, scope(new Scope(nullptr, 0))
		, in_loop(false) {}

	Compound *parse();
};
