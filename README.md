# cc

The least optimized compiler you'll ever see.

## Currently implemented features
- functions and function calls (Sysv ABI)
- variables and global variables
- forward declarations (functions and global variables)
- loops
	- break, continue
- conditionals
- binary operations
- unary operations (++, --, !, ~, -)
- postfix operations (++, --)

## TODO:
- add good error messages
- add octal char, hex char, unicode code point escape codes in lexer
- add power of two optimization for mul, div, mod
- support lvalues not just being identifiers (pointers)
- add comma operator (have to make it lower precedence than =)
	- honestly almost never used
- move function redefinition & param count error checking out of parser
- missing features
	- void, float
	- pointers (arrays, strings)
		- https://en.cppreference.com/w/c/language/pointer
		- pointer type
		- take pointer to (&)
			- for now don't support taking pointer to func args, but later on mark variable as having been moved to stack and move
			- lea -4(%rbp)
			- lea foo(%rip)
		- dereference (\*)
			- load ptr into register, use register as mem addr
- test break after nested for loop
- keep track of pointer depth (char **arr)

char + char = char		no widening
int + char = int		widen rhs
char + int = int		widen lhs
int + int = int			no widening

char a = char b		no widening
int a  = char b		widen rhs
char a = int b		truncate lhs
int a  = int b		no widening

https://norasandler.com/2017/11/29/Write-a-Compiler.html
https://github.com/DoctorWkt/acwj

http://www.quut.com/c/ANSI-C-grammar-y-1999.html
http://www.quut.com/c/ANSI-C-grammar-l-1999.html
