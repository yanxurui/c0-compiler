#pragma once
#include <string>
#include <vector>
#include <map>
#include <set>

using namespace std;
//不能和enum reserver冲突
enum _operator
{
    ADD, SUB, MUL, DIVE, MIN,
    J, JE, JNE, JLT, JLE, JGT, JGE, LABEL,
    ASS, ASS_I,
    PARAMETER, PARAMETER_C, CALL, CALL_VOID, RETURN, SCANF, PRINTF_S, PRINTF_C, PRINTF, SUBSCRIPT, ASS_ARR,
    FUNCTION
};
class Middle
{
    friend class Generator;

    int t;//临时变量的下标从0开始
    int cur;//当前函数的指针，分析完成后，cur应该指向main函数的四元式起始位置
    struct MidCode
    {
        _operator op;
        string op1, op2, res;
        int num;//如果是跳转语句，表示跳转目标（某个中间代码的序号），如果是常量赋值语句则表示右边的整数
    };
    struct Function
    {
        string name;//函数名
        int begin,end;//分别指向该函数的第一个四元式和最后一个四元式之后的位置
        int temp;//临时变量数
    };
    vector<Function> Functions;
    vector<MidCode> MidCodes;
    map<int, int> label_map;//保存跳转指令与跳至的行号
    set<int> labels;//保存跳到的所有行号
public:
    Middle():cur(-1){}
    void new_function(string name);
    //四元式的数量，可以表示下一个四元式的序号
    int size() { return MidCodes.size(); };
    //生成一般的中间代码
    //包括add,sub,mul,dive,minus(MIN),assign(ASS),parameter,call,call_void,return,scanf,printf,printf_s,subscript,ass_arr
    void emit(_operator op, string op1, string op2, string res);
    //生成跳转指令的中间代码
    //包括J, JE, JNE, JLT, JLE, JGT, JGE
    void emit(_operator op, string op1, string op2);
    //生成常量赋值语句,op为ASS_I
    void emit(_operator op, string res, int val);
    //分配临时变量
    string alloc();
    //释放临时变量
    void release(int);
    //设置下标为index的中间代码（跳转指令）的label
    void setLabel(unsigned index, int label);
    void end();
    void output();
private:
    void print(MidCode);
};

//字符串转换成大写形式
inline
string to_uppercase(string &s)
{
    string s2;
    for (unsigned i = 0; i < s.size(); i++)
    {
        s2 = s2 + (char)toupper(s[i]);
    }
    return s2;
}