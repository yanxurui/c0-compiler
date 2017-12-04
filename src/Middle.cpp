#include <iostream>
#include <iomanip>
#include "Middle.h"

void error(string info, int err_no = 0);
string m_to_string(int i);

void Middle::emit(_operator op, string op1, string op2, string res)
{
    MidCode mc;
    mc.op = op;
    mc.op1 = op1;
    mc.op2 = op2;
    mc.res = res;
    MidCodes.push_back(mc);

    //print(mc);
}

void Middle::emit(_operator op, string op1, string op2)
{
    MidCode mc;
    mc.op = op;
    mc.op1 = op1;
    mc.op2 = op2;
    MidCodes.push_back(mc);

    //print(mc);
}
void Middle::emit(_operator, string res, int val)
{
    MidCode mc;
    mc.op = ASS_I;
    mc.res = res;
    mc.num = val;
    MidCodes.push_back(mc);

    //print(mc);
}

string Middle::alloc()
{
    t++;
    if (t >= Functions[cur].temp)//不可能>
        Functions[cur].temp = t+1;
    return m_to_string(t);
}

void Middle::release(int n)
{
    t-=n;
}

void Middle::setLabel(unsigned index, int label)
{
    if (!(index < MidCodes.size()))
        error("Middle::setLabel",3);
    label_map[index] = label;
    labels.insert(label);
}

void Middle::new_function(string name)
{
    Function function;
    function.name = name;
    function.temp = 0;
    Functions.push_back(function);
    cur++;
    t = -1;

    MidCode mc;
    mc.op = FUNCTION;
    mc.op1 = to_uppercase(name);
    MidCodes.push_back(mc);
}

void Middle::end()
{
    //设置跳转语句的标号
    for (auto it = label_map.begin(); it != label_map.end(); it++)
    {
        int label_num = distance(labels.begin(), labels.find(it->second));
        MidCodes[it->first].res = "Lab_" + m_to_string(label_num);
    }
    //插入标号，逆序
    if (labels.size()!=0)
    {
        auto it = labels.end();
        do
        {
            it--;
            MidCode mc;
            mc.op = LABEL;
            mc.op1 = "Lab_" + m_to_string(distance(labels.begin(), it));
            MidCodes.insert(MidCodes.begin() + *it, mc);
        } while (it != labels.begin());
    }
    //统计每个函数对应的四元式起始和结束的位置
    for (int i=0, j=0; j <= cur; i++)
    {
        if (MidCodes[i].op == FUNCTION)
        {
            Functions[j].begin = i;
            if (j != 0)
                Functions[j - 1].end = i;
            j++;
        }
    }
    Functions[cur].end = size();
}

void Middle::output()
{
    cout << left;
    cout << setw(4) << "NO" << setw(10) << "op" << setw(10) << "op1" << setw(10) << "op2" << setw(10) << "res" << endl;

    //for (unsigned i = 0; i <MidCodes.size(); i++)
    //{
    //  MidCode &mc = MidCodes[i];
    //  print(mc);
    //}

    for (int i = 0; i < Functions.size(); i++)
    {
        for (int j = Functions[i].begin; j < Functions[i].end; j++)
        {
            cout << setw(4) << j;
            MidCode &mc = MidCodes[j];
            print(mc);
        }
    }
    cout << right;
}

void Middle::print(MidCode mc)
{
    static const string _operator_str[] =
    {
        "ADD", "SUB", "MUL", "DIVE", "MIN",
        "J", "JE", "JNE", "JLT", "JLE", "JGT", "JGE","LABEL",
        "ASS","ASS_I", "PARAMETER","PARAMETER_C", "CALL", "CALL_VOID", "RETURN", "SCANF", "PRINTF_S","PRINTF_C", "PRINTF", "SUBSCRIPT", "ASS_ARR",
        "FUNCTION"
    };
    //临时变量在前面加个t
    if (mc.op1 != ""&&isdigit(mc.op1[0]))
        mc.op1 = "t" + mc.op1;
    if (mc.op2 != ""&&isdigit(mc.op2[0]))
        mc.op2 = "t" + mc.op2;
    if (mc.res != ""&&isdigit(mc.res[0]))
        mc.res = "t" + mc.res;

    switch (mc.op)
    {
    case ADD:
    case SUB:
    case MUL:
    case DIVE:
    case MIN:

    case PARAMETER:
    case PARAMETER_C:
    case CALL:
    case CALL_VOID:
    case RETURN:
    case SCANF:
    case PRINTF:
    case PRINTF_S:
    case PRINTF_C:
    case SUBSCRIPT:
    case ASS_ARR:
        cout << setw(10) << _operator_str[mc.op] << setw(10) << mc.op1 << setw(10) << mc.op2 << mc.res << endl;
        break;
    case J:
    case JE:
    case JNE:
    case JLT:
    case JLE:
    case JGT:
    case JGE:
        cout << setw(10) << _operator_str[mc.op] << setw(10) << mc.op1 << setw(10) << mc.op2 << setw(10) << mc.res << endl;
        break;
    case ASS:
        cout << setw(10) << _operator_str[mc.op] << setw(10) << mc.op1 << setw(10) << mc.op2 << setw(10) << mc.res << endl;
        break;
    case ASS_I:
        cout << setw(10) << _operator_str[mc.op] << setw(10) << mc.num << setw(10) << mc.op2 << setw(10) << mc.res << endl;
        break;
    case FUNCTION:
        cout << endl;
    case LABEL:
        cout << mc.op1+":" << endl;
        break;
    default:;
    }
}