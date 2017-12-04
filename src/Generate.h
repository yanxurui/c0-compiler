#pragma once
#include <deque>
#include <map>
#include <string>
#include "Table.h"
#include "Middle.h"
using namespace std;

class Generator
{
    Table &table;
    Middle &middle;

    enum Op
    {
        addu, addiu,subu, mult, div, mflo, andi,
        sw, lw, move, li,la,slt,
        j, jal,jr,beq,bne, bltz, blez, bgtz, bgez,sll, syscall,
        label
    };
    //全局数据区，保存全局变量和串
    struct Data
    {
        string id;
        string type;
        string value;
    };
    //mips指令
    struct Instruction
    {
        Op op;
        string op1;
        string op2;
        string op3;
    };
    vector<Data> MipsData;
    vector<Instruction> MipsText;
    
    void code(Op,string op1="",string op2="",string op3="");
    void new_data(string,string,string value="");
    
    //临时寄存器池
    //$t0保留，使用后立即释放
    deque<pair<string, string>> tr_list;//临时寄存器分配列表，保存名字和它对应的临时寄存器名
    //以下两个map都是存储有关临时变量信息的数据结构
    map<string, int> t_overflow;//溢出的临时变量列表，保存临时变量名和它在栈上的地址（偏移量）
    map<string, string> t_save;//临时变量分配到的全局寄存器列表
    
    int offset;//相对$fp的偏移量

    void initial();
    void alloc_sto(int);
    string load(string&);
    string alloc_tr(string&, string addr);
    void save_tr(int n=-1);
    void restore_a(int);
    void new_space(int n);
    void free_space(int n);
    void check_addr(string);
    string get_arr_addr(string &);
public:
    Generator(Table &t, Middle &m) :table(t), middle(m),offset(0) {};
    void generate();
    void output();
    void print(Instruction,ostream&);
};

