#pragma once
#include <string>
#include "Lexer.h"
#include "Table.h"
#include "Middle.h"
using namespace std;
class Parser
{
    Lexer lexer;
    Table &table;
    Middle &middle;
public:
    Parser(string file,Table &t,Middle &m):lexer(file),table(t),middle(m){};
    int analyze();

private:
    int program();
    int const_desc();
    int const_def();
    int integer(int &val);
    int variable_desc();
    int variable_def();

    int function();
    int void_function();
    int main_function();
    int parameter_def();
    int compound_statements();
    int statements();
    int statement();
    int condition_statement();
    int condition();
    int loop();
    int call(string&);
    int call_void();
    int parameter_pass(const string&);
    int assign();
    int read();
    int write();
    int ret();

    Type expression(string&);
    Type term(string&);
    Type factor(string&);
    int is_char();
};