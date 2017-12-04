#include <iostream>
#include <fstream>
#include "Generate.h"

void error(string info, int err_no = 0);
string m_to_string(int i);

//生成一条指令
void Generator::code(Op op, string op1, string op2, string op3)
{
    Instruction instruction;
    instruction.op = op;
    instruction.op1 = op1;
    instruction.op2 = op2;
    instruction.op3 = op3;
    print(instruction, cout);
    MipsText.push_back(instruction);
}

void Generator::new_data(string id, string type, string value)
{
    Data data;
    data.id = id;
    data.type = type;
    data.value = value;
    MipsData.push_back(data);
}

void Generator::generate()
{
    //在静态数据区定义全局变量
    alloc_sto(-1);

    int str_num = 0;//串的编号
    string str_name;//串的名字
    string rs;
    string rt;
    string rd;
    Type t;
    int paras = 0;
    code(j, "MAIN");//代码段的第一条指令
    //以函数为单位生成目标代码
    for (unsigned i = 0; i <middle.Functions.size(); i++)
    {
        Middle::Function function = middle.Functions[i];    
        table.setCursor(i + 1);//指向第i+1个函数的符号表
        initial();//初始化
        code(label, to_uppercase(middle.Functions[i].name));
        
        if (function.name == "main")
        {
            code(move, "fp", "sp");
        }
        else//一、被调用的函数在进入后的准备工作
        {
            new_space(2);
            code(sw, "fp", "4", "sp");//保存帧指针
            code(sw, "ra", "", "sp");//保存返回地址
            code(addiu, "fp", "sp", "8");//活动记录的开始位置
                             //保存8个s寄存器
            new_space(8);
            //for (int i = 0; i < 8; i++)
            //  code(sw, "s" + m_to_string(i), m_to_string(28 - 4 * i), "sp");
            for (int s = 0; s < 8; s++)
                code(sw, "s" + m_to_string(s), m_to_string(-12 - 4 * s), "fp");
        }
        //二、分配存储空间（寄存器和栈）
        alloc_sto(i);

        //三、由四元式生成Mips指令
        for (unsigned j = function.begin + 1; j < function.end; j++)
        {
            Middle::MidCode &mc = middle.MidCodes[j];

            //1.准备好寄存器
            switch (mc.op)
            {
            case ADD:
            case SUB:
            case MUL:
            case DIVE:
                rs = load(mc.op1);
                rt = load(mc.op2);
                rd = load(mc.res);
                break;
            case JE:
            case JNE:
            case JLT:
            case JLE:
            case JGT:
            case JGE:
                rs = load(mc.op1);
                rt = load(mc.op2);
                break;
            case PARAMETER:
            case PARAMETER_C:
            case RETURN:
            case SCANF:
            case PRINTF:
            case PRINTF_C:
                rs = load(mc.op1);
                break;
            default:;
            }
            //2.生成代码
            switch (mc.op)
            {
            case ADD:
                code(addu, rd, rs, rt);
                break;
            case SUB:
                code(subu, rd, rs, rt);
                break;
            case MUL:
                code(mult, rs, rt);
                code(mflo, rd);//取低32位
                break;
            case DIVE:
                code(div, rs, rt);
                code(mflo, rd);//取商
                break;
            case MIN:
                rs = "zero";//使用zero寄存器
                rt = load(mc.op1);
                rd = load(mc.res);
                code(subu, rd, rs, rt);
                break;
            case J:
                code(Op::j, mc.res);
                break;
            case JE:
                code(beq, rs, rt, mc.res);
                break;
            case JNE:
                code(bne, rs, rt, mc.res);
                break;
            case JLT:
                code(subu, "t0", rs, rt);
                code(bltz, "t0", mc.res);
                break;
            case JLE:
                code(subu, "t0", rs, rt);
                code(blez, "t0", mc.res);
                break;
            case JGT:
                code(subu, "t0", rs, rt);
                code(bgtz, "t0", mc.res);
                break;
            case JGE:
                code(subu, "t0", rs, rt);
                code(bgez, "t0", mc.res);
                break;
            case LABEL:
                code(label, mc.op1);
                break;
            case ASS:
                rs = load(mc.op1);
                rd = load(mc.res);
                if (table.getType(mc.res) == CHAR)
                {
                    code(andi, rs, rs, "0xff");
                }
                //code(addu, rd, rs, "zero");
                code(move, rd, rs);
                break;
            case ASS_I:
                rd = load(mc.res);
                code(li, rd, m_to_string(mc.num));
                break;
            case ASS_ARR:
                rs = load(mc.op2);//下标

                //增加
                code(slt, "t0", rs, "zero");
                code(bne, "t0", "zero", "ERROR1");
                code(li, "t0", m_to_string(table.getValue(mc.op1)));
                code(slt, "t0", rs, "t0");
                code(beq, "t0", "zero", "ERROR1");
                //
                code(sll, rs, rs, "2");
                code(addu,rs,rs,get_arr_addr(mc.op1));//获取数组元素的地址
                rd = load(mc.res);
                if (table.getType(mc.op1) == CHAR)
                {
                    code(andi, rd, rd, "0xff");
                }
                code(sw, rd, "",rs);//赋值
                break;
            case SUBSCRIPT:
                rs = load(mc.op2);//下标

                //增加
                code(slt, "t0", rs, "zero");
                code(bne, "t0", "zero", "ERROR1");
                code(li, "t0", m_to_string(table.getValue(mc.op1)));
                code(slt, "t0", rs, "t0");
                code(beq, "t0", "zero", "ERROR1");
                //
                code(sll, rs, rs, "2");
                code(addu, rs, rs, get_arr_addr(mc.op1));//获取数组元素的地址
                rd = load(mc.res);
                code(lw, rd,"", rs);//取值
                break;
            case PARAMETER:
            case PARAMETER_C:
                if (paras == 0)//先保存临时寄存器
                {
                    save_tr();
                }
                if (paras < 4)//保存参数寄存器
                {
                    rd = "a" + m_to_string(paras);
                    new_space(1);
                    code(sw, rd, "", "sp");
                    if (mc.op == PARAMETER_C)
                    {
                        code(andi, rs, rs, "0xff");
                    }
                    code(move, rd, rs);
                }
                else//压入栈
                {
                    new_space(1);
                    if (mc.op == PARAMETER_C)
                    {
                        code(andi, rs, rs, "0xff");
                    }
                    code(sw, rs, "","sp");
                }
                paras++;
                break;
            case CALL:
                if (paras == 0)
                    save_tr();
                code(jal, to_uppercase(mc.op1));
                if (paras > 4)//先释放栈上的参数
                    free_space(paras - 4);
                if (paras > 0)//恢复参数寄存器的值
                {
                    paras = paras > 4 ? 4 : paras;
                    restore_a(paras);
                }
                //restore_t();
                rd=load(mc.res);
                code(move, rd, "v0");//获取返回值
                paras = 0;
                break;
            case CALL_VOID:
                if (paras == 0)
                    save_tr();
                code(jal, to_uppercase(mc.op1));
                if (paras > 4)//先释放栈上的参数
                    free_space(paras - 4);
                if (paras > 0)//恢复参数寄存器的值
                {
                    paras = paras > 4 ? 4 : paras;
                    restore_a(paras);
                }
                //restore_t();
                paras = 0;
                break;
            case RETURN:
                rs = load(mc.op1);
                if (table.getType(middle.Functions[i].name) == CHAR)
                    code(andi, rs, rs, "0xff");
                code(move, "v0", rs);
                break;
            case SCANF:
                t= table.getType(mc.op1);
                if (t == INT)//Read Integer
                {
                    rd = load(mc.op1);
                    code(li, "v0", "5");
                    code(syscall);
                    code(move, rd, "v0");
                }
                else if (t == CHAR)//Read Char
                {
                    rd = load(mc.op1);
                    code(li, "v0", "12");
                    code(syscall);
                    code(move, rd, "v0");
                }
                else
                    error("scanf语句中的变量不是int也不是char",3);
                break;
            case PRINTF://Print Integer
                new_space(1);
                code(sw, "a0", "", "sp");//保存$a0的值
                code(move, "a0", rs);
                code(li, "v0", "1");
                code(syscall);
                code(lw, "a0", "", "sp");//恢复$a0的值
                free_space(1);
                break;
            case PRINTF_S://Print String
                str_name = "str" + m_to_string(++str_num);
                new_data(str_name,"ASCIIZ",mc.op1);
                new_space(1);
                code(sw, "a0", "", "sp");//保存$a0的值
                code(la, "a0", str_name);
                code(li, "v0", "4");
                code(syscall);
                code(lw, "a0", "", "sp");//恢复$a0的值
                free_space(1);
                break;
            case PRINTF_C://Print Char
                new_space(1);
                code(sw, "a0", "", "sp");//保存$a0的值
                code(move, "a0", rs);
                code(li, "v0", "11");
                code(syscall);
                code(lw, "a0", "", "sp");//恢复$a0的值
                free_space(1);
                break;
            default:;
            }
        }
        //四、被调用的函数在退出前恢复相应的寄存器，该过程一般跟在return语句之后
        if (function.name != "main")
        {
            //将保存在临时寄存器里的全局变量的值写回
            save_tr(0);
            //恢复8个s寄存器
            for (int s = 0; s < 8; s++)
                code(lw, "s" + m_to_string(s), m_to_string(-12 - 4 * s),"fp");
            code(lw, "ra", "-8", "fp");//恢复$ra
            code(move, "sp", "fp");//恢复$sp
            code(lw, "fp", "-4", "fp");//恢复$fp

            code(jr, "ra");//返回
        }
    }
    code(j, "END");

    //越界
    code(label, "ERROR1");
    str_name = "str" + m_to_string(++str_num);
    new_data(str_name, "ASCIIZ", "Array out of range");
    code(la, "a0", str_name);
    code(li, "v0", "4");
    code(syscall);
    code(j, "END");

    code(label, "END");
}

//初始化
void Generator::initial()
{
    offset = 0;
    tr_list.clear();
    t_overflow.clear();
    t_save.clear();
}

//存储分配
void Generator::alloc_sto(int m)
{
    //m=-1在数据段定义全局变量
    if (m == -1)
    {
        vector<string> &idents = table.ident_lists[0];
        int n = idents.size();
        for (int j = 0; j < n; j++)
        {
            string& ident = idents[j];
            Kind k=UNKNOW;
            table.lookup(ident, k);
            if(k==VAR)
            {
                //string type = table.getType(ident) == CHAR ? "BYTE" : "WORD";
                new_data(ident, "WORD","-1");
            }
            else if (k == ARR)
            {
                int length = table.getValue(ident);
                new_data(ident, "WORD", "-1:"+m_to_string(length));
            }

        }
        return;
    }

    //全局寄存器依次分配给前4个临时变量（最活跃）和局部变量，未分配到全局寄存器的局部变量保存在栈中，同时填符号表
    int s;//全局寄存器的编号
    //1.临时变量
    int temp = middle.Functions[m].temp;
    for ( s= 0; s <temp &&s < 4; s++)
    {
        t_save[m_to_string(s)] = "s" + m_to_string(s);
    }
    //2.局部变量
    vector<string> &idents = table.ident_lists[m+1];
    int para_num = table.getParas(middle.Functions[m].name).size();
    int j;
    //设置第4个以后的参数的地址
    if (para_num > 4)
    {
        for (j = 4; j < para_num; j++)
        {
            int addr = 4 * (para_num - j-1);
            table.setAddr(idents[j], m_to_string(addr));
        }
    }
    for (j=para_num; j < idents.size(); j++)
    {
        string& ident = idents[j];
        Kind k = UNKNOW;
        table.lookup(ident, k);
        if (s<8&&k == VAR)//有全局寄存器并且是局部简单变量
        {
            table.setAddr(ident, "s" + m_to_string(s));
            s++;
        }
        else//3.未分配到全局寄存器的局部变量和数组
        {
            if (k == VAR)
            {
                new_space(1);
                table.setAddr(ident, m_to_string(offset));
            }
            else if (k == ARR)//数组是倒着存的
            {
                new_space(table.getValue(ident));
                table.setAddr(ident, m_to_string(offset));
            }
        }
    }

    if (s < 8 && temp>4)//如果还有未使用的全局寄存器并且还有未分配的临时变量
    {
        for (j = 4; j < temp &&s < 8; j++, s++)
        {
            t_save[m_to_string(j)] = "s" + m_to_string(s);
        }
    }
}

//根据给定的标识符名字返回它的位置（寄存器）
//输入的参数可能是临时变量、局部变量（包括参数，数组）、全局变量
string Generator::load(string &id)
{
    if (isdigit(id[0]))//临时变量，可能保存在s寄存器，也可能保存在t寄存器
    {
        auto it = t_save.find(id);
        if (it != t_save.end())
            return it->second;
        return alloc_tr(id,"-2");
    }

    string addr = table.getAddr(id);
    if (addr[0] == 's' || addr[0] == 'a')//保存在寄存器中
        return addr;
    return alloc_tr(id, addr);//其他，保存在数据区或栈上
}

//给变量分配临时寄存器
string Generator::alloc_tr(string &id, string addr)
{
    //淘汰策略是最久未使用法：每次将被访问的名字放到队头，淘汰的时候选择最后一个移除
    string pos;

    auto it = tr_list.begin();
    while (it != tr_list.end())
    {
        if (it->first == id)
            break;
        it++;
    }

    //1.已经存在，返回它的寄存器编号
    if (it != tr_list.end())
    {
        pos = it->second;
        tr_list.erase(it);
        //为了防止失效，先删除再插入
        tr_list.push_front(make_pair(id, pos));
        return "t" + pos;
    }

    //2.分配新的临时寄存器

    if (tr_list.size() < 9)
    {
        pos = m_to_string(tr_list.size()+1);
    }
    else//没有可用临时寄存器，先溢出 
    {
        pos = tr_list[8].second;
        save_tr(1);
    }

    if (addr == "-1")//全局变量
    {
        code(la, "t0", id);
        code(lw, "t" + pos, "", "t0");
    }
    else if (addr == "-2")//临时变量
    {
        //可能是被溢出到栈上了，重新加载到寄存器
        auto it2 = t_overflow.find(id);
        if (it2 != t_overflow.end())
        {
            int addr2 = it2->second;
            code(lw, "t" + pos, m_to_string(addr2), "fp");
        }
    }
    else//局部变量
    {
        check_addr(addr);
        code(lw, "t" + pos, addr, "fp");
    }

    tr_list.push_front(make_pair(id, pos));

    return "t" + pos;
}

//保存临时寄存器$t1~$t9的值
//n=-1：保存所有临时寄存器（默认），n=0,：只保存全局变量，n=1：溢出最久未使用的临时寄存器
void Generator::save_tr(int n)
{
    auto it = tr_list.begin();
    if (n == 1)
        it = it + 8;//指向最后一个元素
    for (; it != tr_list.end(); it++)
    {
        string id = it->first;
        if (isdigit(id[0])&&n!=0)//临时变量
        {
            auto it2 = t_overflow.find(id);
            //如果已经在栈上分配过空间了
            if (it2 != t_overflow.end())
            {
                code(sw, "t" + it->second, m_to_string(it2->second), "fp");
            }
            else
            {
                new_space(1);
                code(sw, "t" + it->second, "", "sp");
                t_overflow[id] = offset;//相对地址
            }
        }
        else
        {
            string addr = table.getAddr(id);
            if (addr == "-1")//如果是全局变量，写回静态数据段
            {
                code(la, "t0", id);
                code(sw, "t" + it->second, "", "t0");
            }
            else if(n!=0)//写回栈
            {
                check_addr(addr);
                code(sw, "t" + it->second, addr, "fp");
            }
        }
    }
    if (n == 1)
        tr_list.pop_back();
    else if (n == -1)
        tr_list.clear();
}

void Generator::restore_a(int paras)
{
    for (int i = 0; i < paras; i++)
        code(lw, "a" + m_to_string(i), m_to_string(4 * (paras - i - 1)), "sp");
    free_space(paras);
}

//在栈上开辟n个字的空间
void Generator::new_space(int n)
{
    n *= 4;
    n = -n;
    offset += n;
    code(addiu, "sp", "sp",m_to_string(n));
}

//在栈上释放n个字的空间
void Generator::free_space(int n)
{
    n *= 4;
    offset += n;
    code(addiu, "sp", "sp", m_to_string(n));
}

//检查一个地址是否是栈上相对$fp的偏移地址
void Generator::check_addr(string addr)
{
    int addr2 = stoi(addr);
    if (addr2 % 4 )
    {
        error("check_addr:错误的地址",3);
    }
}
//获取数组的起始地址
string Generator::get_arr_addr(string &arr_name)
{
    string addr = table.getAddr(arr_name);
    if (addr == "-1")//全局数组
        code(la, "t0", arr_name);
    else
    {
        check_addr(addr);
        code(addiu, "t0", "fp", addr);
    }
    return "t0";
}

void Generator::output()
{
    //1.定义输出流
    ofstream fout;
    fout.open("mips.S");
    if (!fout)
        error("无法打开输出文件", 3);
    //2.输出数据定义指令
    fout << ".data" << endl;
    for (unsigned i = 0; i < MipsData.size(); i++)
    {
        Data& data = MipsData[i];
        fout << data.id << ":" << "\t." << data.type;
        if (data.value != "")
        {
            if(data.type=="ASCIIZ")
                fout << "\t\"" << data.value<<"\"";
            else fout << "\t" << data.value;
        }
        fout << endl;
    }
    fout << endl;
    //3.输出代码段指令
    fout << ".text" << endl;
    for (unsigned i = 0; i < MipsText.size(); i++)
    {
        Instruction is = MipsText[i];
        print(is, fout);
    }
}

void Generator::print(Instruction is, ostream &os)
{
    string Op_str[] =
    { "addu", "addiu","subu", "mult", "div", "mflo","andi",
    "sw", "lw", "move", "li","la","slt",
    "j", "jal","jr","beq","bne", "bltz", "blez", "bgtz", "bgez","sll", "syscall",
    "label"
    };
    switch (is.op)
    {
    case mflo:
    case jr:
        os << Op_str[is.op] << "\t$" << is.op1 << endl;
        break;
    case sw:
    case lw:
        os << Op_str[is.op] << "\t$" << is.op1 << "," << is.op2 << "($" << is.op3 << ")" << endl;
        break;
    case mult:
    case div:
    case move:
        os << Op_str[is.op] << "\t$" << is.op1 << ",$" << is.op2 << endl;
        break;
    case li:
    case la:
    case bltz:
    case blez:
    case bgtz:
    case bgez:
        os << Op_str[is.op] << "\t$" << is.op1 << "," << is.op2 << endl;
        break;
    case j:
    case jal:
        os << Op_str[is.op] << "\t" << is.op1<< endl;
        break;
    case beq:
    case bne:
    case sll:
    case addiu:
    case andi:
        os << Op_str[is.op] << "\t$" << is.op1 << ",$" << is.op2 << "," << is.op3 << endl;
        break;
    case syscall:
        os << "syscall" << endl;
        break;
    case label:
        os << is.op1 + ":" << endl;
        break;
    case addu:
    case subu:
    case slt:
        os << Op_str[is.op] << "\t$" << is.op1 << ",$" << is.op2 << ",$" << is.op3 << endl;
        break;
    default:error("Generator::print 无法识别的mips指令", 3);
    }
}
