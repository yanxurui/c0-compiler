#pragma once

#include <string>
#include <vector>
#include <unordered_map>

using namespace std;
//符号表表项的种类
enum Kind
{
    VAR, CON, ARR, PARA,FUN, V_FUN,UNKNOW
};
//符号表表项的类型
enum Type
{
    INT, CHAR, NULL_TYPE
};
class Table
{   
    struct table_value
    {
        Kind kind;
        Type type;
        int value;//1.常量的值2.函数的私有符号表的索引3.数组的维数
        string addr;
    };
    
    typedef unordered_map<string, table_value> table;
    //符号表
    //每个函数一个符号表，下标为0的符号表是全局符号表
    vector<table> tables;
    
    int cur;//当前符号表指针
public:
    Table();
    //填表，成功返回1，名字冲突返回0
    //如果插入的是函数名，则自动新建一个符号表
    int insert(string name, Kind k, Type t = NULL_TYPE, int val = 0);
    //查找指定种类的名字是否存在，如果指定的种类为UNKNOW，则同时返回种类
    //首先在当前符号表中查找，然后在全局符号表中查找，找到返回1，否则返回0
    int lookup(string name,Kind &k);
    //获取常量的数值
    int getValue(string);
    //获取函数的参数个数
    vector<Type> getParas(const string&);
    //查找标识符的类型，可能为INT,CHAR或NULL_TYPE
    Type getType(const string&);
    //对局部变量（包括数组）填地址
    //参数为s寄存器或在栈中相对活动记录的基的偏移量
    void setAddr(string&,string);
    //查询地址，参数可能是局部变量（包括数组、参数）
    string getAddr(string&);
    //设置符号表指针cur的值
    void setCursor(int);
    vector<vector<string>> ident_lists;
private:
    void create_table();
    //在当前符号表中查找名字
    int lookup_local(string name);
    int lookup_global(string name);
};