#include <iostream>
#include "Table.h"
#include "Middle.h"
#include "Lexer.h"
#include "Parser.h"
#include "Generate.h"
using namespace std;

extern string token;
extern int errors;

void test_lexer();
void test_lexer2();
void test_middle();
void test_table();
void test_parser();
void test_generate();
void final_test();
int main()
{
    final_test();
}
void test_lexer()
{
    cout << "***************Test Lexer***************" << endl;
    Lexer lexer("test_lexer.txt");
    lexer.getsym();
    cout << token<<endl;
    lexer.getsym();
    lexer.rollback(1);
    cout << token << endl;
    lexer.getsym();
    lexer.getsym();
    lexer.rollback(2);
    lexer.output();
}
void test_lexer2()
{
    cout << "***************Test Lexer2 错误处理测试***************" << endl;
    Lexer lexer("test_lexer2.txt");
    lexer.output();
}
void test_middle()
{
    cout << "***************Test Middle***************" << endl;
    Middle middle;
    string op1 = middle.alloc();
    string op2 = middle.alloc();
    middle.release(2);
    string res = middle.alloc();
    middle.emit(ADD, op1, op2, res);//加
    middle.emit(MIN, res, "", res);//负号
    string res2 = middle.alloc();
    //下面两条四元式都是给res2赋值
    middle.emit(ASS, res, "", res2);//赋值
    middle.emit(ASS_I, res2,100);//常量赋值语句
    string fn = "function";
    string res3 = middle.alloc();
    middle.emit(CALL, fn, "", res3);//有返回值函数调用
    middle.emit(CALL_VOID, fn, "", "");//无返回值函数调用
    op1 = middle.alloc();
    middle.emit(RETURN, op1, "", "");
    middle.emit(SCANF, op1, "", "");
    middle.emit(PRINTF_S, "hello", "", "");
    middle.emit(PRINTF, op1, "", "");
    string arr = "A";
    middle.emit(SUBSCRIPT, arr, op1, res3);
    op2 = middle.alloc();
    middle.emit(ASS_ARR, arr, op1, op2);//对数组元组赋值
}

void test_table()
{
    cout << "***************Test Table***************" << endl;
    Table table;
    Kind k;
    table.insert("zero", CON, INT, 0);
    table.insert("var", VAR, INT);
    table.insert("A", ARR, CHAR, 10);
    table.insert("fn", FUN, INT);
    if (!table.insert("var", PARA,  INT))//允许
        cout << "error" << endl;
    k= UNKNOW;
    if (table.lookup("var", k) && (k == PARA))//?
        cout << "局部的名字隐藏了同名的全局名字" << endl;
    table.insert("fn2", V_FUN);
    table.insert("ch", PARA, CHAR);
    table.insert("pa2", PARA, CHAR);
    if(table.insert("ch", VAR, CHAR))//不允许
        cout<<"error"<<endl;
    table.insert("one", CON, CHAR, '1');
    table.insert("fn2", FUN,INT);//不允许

    
    k = VAR;
    if (!table.lookup("ch", k))
        cout << "局部的名字不能重名" << endl;
    k = FUN;
    if (!table.lookup("fn2", k))
        cout << "全局的名字也不能重名" << endl;
    if ((table.getValue("zero") == 0)&&(table.getValue("one")==49))
        cout << "可以获取常量的值" << endl;
    if ((table.getParas("fn").size() == 1) && (table.getParas("fn2").size() == 2))
        cout << "可以获取函数的参数个数" << endl;
}

void test_parser()
{
    Table table;
    Middle middle;
    Parser parser("test_parser.txt", table, middle);
    parser.analyze();
    cout << "***************四元式***************" << endl;
    middle.output();
}

void test_generate()
{
    Table table;
    Middle middle;
    Parser parser("test_generate.txt", table, middle);
    parser.analyze();
    cout << "***************四元式***************" << endl;
    middle.output();
    if (errors)
        exit(0);
    Generator generator(table, middle);
    cout << "***************generate code***************" << endl;
    generator.generate();
    generator.output();
    cout << "see mips.S" << endl;
}
void final_test()
{
    string file;
    cout << "file directory:";
    cin >> file;

    Table table;
    Middle middle;
    Parser parser(file, table, middle);
    parser.analyze();
    cout << "***************四元式***************" << endl;
    middle.output();
    if (errors)
        exit(0);
    Generator generator(table, middle);
    cout << "***************generate code***************" << endl;
    generator.generate();
    generator.output();
    cout << endl << "see mips.S" << endl;
}