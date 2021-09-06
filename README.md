# cc

The least optimized compiler you'll ever see.

TODO:
- add good error messages
- add octal char, hex char, unicode code point escape codes in lexer
- add power of two optimization for mul, div, mod
- support lvalues not just being identifiers
- add comma operator (have to make it lower precedence than =)
	- honestly almost never used
- move function redefinition + param count error checking out of parser
- missing features
	- char, void, float
	- arrays
	- pointers
- add separate datatype for global variables

https://norasandler.com/2017/11/29/Write-a-Compiler.html

http://www.quut.com/c/ANSI-C-grammar-y-1999.html
http://www.quut.com/c/ANSI-C-grammar-l-1999.html
