#pragma once
#include <string>
#include <map>
#include <fstream>

using namespace std;
//类别码
enum CLASS
{
    IDEN, INTCON, ZERO,CHARCON, STRCON,
    CONSTTK, INTTK, CHARTK, VOIDTK,
    MAINTK, IFTK, ELSETK, WHILETK, FORTK, SCANFTK, PRINTFTK, RETURNTK,
    PLUS, MINU, MULT, DIV, LSS, LEQ, GRE, GEQ, EQL, NEQ, ASSIGN,
    SEMICN, COMMA, LPARENT, RPARENT, LBRACK, RBRACK, LBRACE, RBRACE,
    NULL_CLASS
};

class Lexer
{
    //保留字的字符串表示
    map<string, CLASS> reserver;
    char ch;//当前读进的字符0~255
    int r_char;//回退字符标记
    int r_token;//回退单词标记
    
    bool j;//是否跳过当前正在分析的单词
    //int pos[2];//记录当前读到的字符的位置，分别为行号和列号
    ifstream fin;//输入的源文件

    //维护一个循环队列，保存最近max个单词的信息
    class Rollback
    {
        static const int max = 5;//最大单词回退数
        CLASS S[max];
        string T[max];
        unsigned N[max];
        int head;
    public:
        Rollback():head(-1){}
        void push(CLASS, string, int);
        void get(int i, CLASS&, string&, int&);
    }rb;

public:

    Lexer(string file_name);
    //获取下一个单词
    void getsym();
    //回退x个单词，x最大值为3，连续回退的数量也不超过3
    void rollback(int n);

    void output();

private:
    void getch();
    void retract();
};
inline
bool isLetter(char ch)
{
    return isalpha(ch) || ch == '_';
}
inline
bool isAdd(char ch)
{
    return ch == '+' || ch == '-';
}
inline
bool isMul(char ch)
{
    return ch == '*' || ch == '/';
}
inline
bool isChar(char ch)
{
    return ch == 32 || ch == 33 || ch >= 35 && ch <= 126;
}