(gdb) bt
#0  0x00007f4f602175c1 in __memmove_avx_unaligned_erms () from /usr/lib/libc.so.6
#1  0x00007f4f60522066 in std::char_traits<char>::copy (__n=4, __s2=<optimized out>, __s1=<optimized out>)
    at /build/gcc/src/gcc-build/x86_64-pc-linux-gnu/libstdc++-v3/include/bits/char_traits.h:409
#2  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::_S_copy (__n=4, __s=<optimized out>, 
    __d=<optimized out>) at /build/gcc/src/gcc-build/x86_64-pc-linux-gnu/libstdc++-v3/include/bits/basic_string.h:351
#3  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::_S_copy (__n=4, __s=<optimized out>, 
    __d=<optimized out>) at /build/gcc/src/gcc-build/x86_64-pc-linux-gnu/libstdc++-v3/include/bits/basic_string.h:346
#4  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::_M_assign (this=this@entry=0x7fffca6e2ee0, 
    __str="main") at /build/gcc/src/gcc-build/x86_64-pc-linux-gnu/libstdc++-v3/include/bits/basic_string.tcc:272
#5  0x00007f4f605223ff in std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::assign (__str=..., 
    this=0x7fffca6e2ee0) at /build/gcc/src/gcc-build/x86_64-pc-linux-gnu/libstdc++-v3/include/bits/basic_string.h:1342
#6  std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::operator= (this=0x7fffca6e2ee0, __str=...)
    at /build/gcc/src/gcc-build/x86_64-pc-linux-gnu/libstdc++-v3/include/bits/basic_string.h:667
--Type <RET> for more, q to quit, c to continue without paging--
#7  0x000056501c3d6730 in Token::Token (this=0x7fffca6e2ed0, t=...) at include/lexer.hpp:83
#8  0x000056501c3d6408 in Lexer::peek (this=0x7fffca6e3130, lookahead=1) at src/lexer.cpp:454
#9  0x000056501c3d6329 in Lexer::peek_next (this=0x7fffca6e3130) at src/lexer.cpp:444
#10 0x000056501c3d48dd in Lexer::eat (this=0x7fffca6e3130, expected=IDENTIFIER) at src/lexer.cpp:67
#11 0x000056501c3d8ce1 in Parser::parse (this=0x7fffca6e3120) at src/parser.cpp:144
#12 0x000056501c3d9bb2 in main (argc=2, argv=0x7fffca6e32d8) at main.cpp:14