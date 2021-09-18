#pragma once

#include <string>

#include <lexer.hpp>

void err_tok(const std::string &msg, Token t);
void err(const std::string &msg);
void warning(const std::string &msg);
