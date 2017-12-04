#include <string>
#include "Parser.h"
using namespace std;

extern CLASS sym;//枚举型的单词类别码
extern string token;//标识符名字
extern int num;//无符号整数或字符的值
//err_no=0，表示缺少某个符号
//err_no=1，编译停止
void error(string info, int err_no = 0);

int Parser::analyze()
{
    return program();
}
//＜程序＞::= ［＜常量说明＞］［＜变量说明＞］{＜有返回值函数定义＞|＜无返回值函数定义＞}＜主函数＞
int Parser::program()
{
    lexer.getsym();
    
    const_desc();
    variable_desc();
    lexer.rollback(1);
    do
    {
        lexer.getsym();
        if (sym == INTTK || sym == CHARTK)//只可能是有返回值函数定义
            function();
        else if (sym == VOIDTK)//遇到void，可能是无返回值函数或main函数定义
        {
            lexer.getsym();
            if (sym == MAINTK)
                break;
            else if(sym==IDEN)
                void_function();
            else error("不是一个函数名",1);
        }
        else error("错误的函数定义", 1);
    } while (true);
    //如果没有main函数会报错
    //不分析main函数之后的内容
    main_function();
    middle.end();
    return 1;
}
//＜常量说明＞ :: = const＜常量定义＞; { const＜常量定义＞; }
//进入前向前多读一个
//退出后向前多读一个单词
int Parser::const_desc()
{
    int n = 0;
    while (sym == CONSTTK)
    {
        const_def();
        lexer.getsym();
        if (sym != SEMICN)
        {
            lexer.rollback(1);
            error("缺少;");
        }
        n++;
        lexer.getsym();
    }
    return n;
}
//＜常量定义＞::=   int＜标识符＞＝＜整数＞{,＜标识符＞＝＜整数＞}
//                                  | char＜标识符＞＝＜字符＞{ ,＜标识符＞＝＜字符＞ }
int Parser::const_def()
{
    string name;
    int value;
    lexer.getsym();
    if (sym == INTTK)
    {
        do
        {
            lexer.getsym();
            if (sym != IDEN)
                error("常量定义缺少标识符",1);
            name = token;
            lexer.getsym();
            if (sym != ASSIGN)
                error("常量定义缺少=",1);
            if(!integer(value))
                error("int型常量定义的右边缺少整数",1);
            table.insert(name, CON, INT, value);
            lexer.getsym();
        } while (sym == COMMA);
    }
    else if(sym==CHARTK)
    {
        do
        {
            lexer.getsym();
            if (sym != IDEN)
                error("常量定义缺少标识符", 1);
            name = token;
            lexer.getsym();
            if (sym != ASSIGN)
                error("常量定义缺少=", 1);
            lexer.getsym();
            if (sym != CHARCON)
                error("char型常量定义的右边缺少字符", 1);
            value = num;
            table.insert(name, CON, CHAR, value);
            lexer.getsym();
        } while (sym == COMMA);
    }
    else error("const后面缺少类型标识符" ,1);
    lexer.rollback(1);
    return 1;
}
//＜整数＞::= ［＋｜－］＜无符号整数＞｜０
int Parser::integer(int &val)
{
    lexer.getsym();
    if (sym == ZERO)
        val=0;
    else if (sym == INTCON)
        val = num;
    else if (sym == PLUS || sym == MINU)
    {
        int minus=sym==MINU?1:0;
        lexer.getsym();
        if (sym == INTCON)
            if(minus)
                val= -num;
            else val = num;
        else
        {
            error("+或-后面的"+token+"不是一个无符号整数");//暂时不终止，由调用方决定
            return 0;
        }
    }
    else//error
    {
        return 0;
    }
    return 1;
}
//＜变量说明＞::= ＜变量定义＞;{＜变量定义＞;} //带有试探性，可能会遇到有返回值的函数定义
//进入前向前多读一个
//退出后向前多读一个单词
int Parser::variable_desc()
{
    int n = 0;
    while (sym == INTTK || sym == CHARTK)
    {
        lexer.getsym();
        if (sym != IDEN)
            error("", 1);//也可能是函数名定义错误
        lexer.getsym();
        //超前扫描判断是否为标识符，如果不是就退出。并且会退到进入之前的状态
        if (sym == LBRACK || sym == COMMA || sym == SEMICN|| sym != LPARENT)
        {//当sym!=LPARENT时，不是有返回值函数定义，应该为变量定义，缺少,或;
            lexer.rollback(2);
            variable_def();
            lexer.getsym();
            if (sym != SEMICN)
            {
                lexer.rollback(1);
                error("缺少;");
            }
            n++;
            lexer.getsym();
        }
        else
        {
            lexer.rollback(2);
            break;
        }
    }
    return n;
}

//＜变量定义＞  ::=＜类型标识符＞(＜标识符＞|＜标识符＞‘[’＜无符号整数＞‘]’){,(＜标识符＞|＜标识符＞‘[’＜无符号整数＞‘]’ )}
//进入前多读1个单词
//退出后也向前多读1个单词
int Parser::variable_def()
{
    string name;
    Type t = sym == INTTK ? INT : CHAR;
    do
    {
        lexer.getsym();
        if (sym != IDEN)
        {
            //if (sym == INTTK || sym == CHARTK)
            //{
            //  //error("缺少;");//;误写成,
            //  lexer.rollback(1);
            //  return 1;
            //}
            //else error("缺少标识符", 1);
            error("变量定义没有结束或缺少;", 1);
        }
        
        name = token;
        lexer.getsym();
        if (sym == LBRACK)//数组
        {
            lexer.getsym();
            if (sym != INTCON)//无符号整数
                error("数组定义的[后面缺少无符号整数", 1);
            table.insert(name, ARR, t, num);
            lexer.getsym();
            if (sym != RBRACK)
            {
                lexer.rollback(1);
                error("数组定义缺少]");
            }
            lexer.getsym();
        }
        else if(sym==COMMA)//变量定义尚未结束
            table.insert(name, VAR, t);
        else if (sym == SEMICN)//变量定义结束
        {
            table.insert(name, VAR, t);
            break;
        }
        else
        {
            table.insert(name, VAR, t);//当做缺少;处理
            break;
        }
    } while (sym==COMMA);
    lexer.rollback(1);
    return 1;
}
//＜有返回值函数定义＞::=＜声明头部＞‘(’＜参数＞‘)’  ‘{’＜复合语句＞‘}’
//进入前先读1个单词
int Parser::function()
{
    Type t= sym==INTTK?INT:CHAR;
    lexer.getsym();
    if (sym != IDEN)
        error("缺少函数名",1);
    table.insert(token, FUN, t);
    middle.new_function(token);
    lexer.getsym();
    if (sym != LPARENT)
        error("缺少(",1);
    parameter_def();
    lexer.getsym();
    if (sym != RPARENT)
    {
        lexer.rollback(1);
        error("有返回值函数定义缺少)");
    }
    lexer.getsym();
    if (sym != LBRACE)
        error("缺少{",1);
    compound_statements();
    lexer.getsym();
    if (sym != RBRACE)
    {
        lexer.rollback(1);
        error("缺少}");
    }
    return 1;
}

//＜无返回值函数定义＞  ::= void＜标识符＞‘(’＜参数＞‘)’  ‘{’＜复合语句＞‘}’
//进入前先读2个单词
int Parser::void_function()
{
    table.insert(token, V_FUN);
    middle.new_function(token);
    lexer.getsym();
    if (sym != LPARENT)
        error("缺少(",1);
    parameter_def();
    lexer.getsym();
    if (sym != RPARENT)
    {
        lexer.rollback(1);
        error("无返回值函数定义缺少)");
    }
    lexer.getsym();
    if (sym != LBRACE)
        error("缺少{",1);
    compound_statements();
    lexer.getsym();
    if (sym != RBRACE)
    {
        lexer.rollback(1);
        error("缺少}");
    }
    return 1;
}
//＜主函数＞::= void main ‘(’ ‘)’  ‘{’＜复合语句＞‘}’
//进入前先读2个
int Parser::main_function()
{
    table.insert(token, V_FUN);
    middle.new_function(token);
    lexer.getsym();
    if (sym != LPARENT)
        error("缺少(",1);
    lexer.getsym();
    if (sym != RPARENT)
    {
        lexer.rollback(1);
        error("缺少)");
    }
    lexer.getsym();
    if (sym != LBRACE)
        error("缺少{", 1);
    int flag = 0;
    do 
    {
        compound_statements();
        lexer.getsym();
        if (sym != RBRACE)//如果变量定义没有出现在函数开始，也会出现这种情况。
        {
            if (const_desc() || variable_desc())
            {
                flag = 1;
                lexer.rollback(1);
                error("常量或变量说明必须位于函数开始处");
            }
            else
            {
                flag = 0;
                lexer.rollback(1);
                error("缺少}");
            }
        }
        else
            flag = 0;
    } while (flag);
    return 1;
}
//＜参数表＞::=＜类型标识符＞＜标识符＞{,＜类型标识符＞＜标识符＞}| ＜空＞
int Parser::parameter_def()
{
    Kind k = PARA;
    Type t=NULL_TYPE;//必须要初始化
    int f = 0;
    lexer.getsym();
    if (sym != RPARENT)
    {
        lexer.rollback(1);
        do
        {
            lexer.getsym();
            if (!(sym == INTTK || sym == CHARTK))
            {
                if (sym == IDEN)//如果后面是标识符，则继续
                {
                    lexer.rollback(1);
                    error("参数说明缺少类型说明");
                }
                else if (sym == LBRACE)//说明无惨函数缺少)//继续
                    break;
            }
            else t = sym == INTTK ? INT : CHAR;
            lexer.getsym();
            if (sym != IDEN)
                error("参数说明缺少标识符",1);
            table.insert(token, k, t);
            lexer.getsym();
        } while (sym == COMMA);
    }
    lexer.rollback(1);
    return 1;
}
//＜复合语句＞::=  ［＜常量说明＞］［＜变量说明＞］＜语句列＞
int Parser::compound_statements()
{
    lexer.getsym();
    const_desc();
    variable_desc();
    lexer.rollback(1);
    statements();
    return 1;
}
//＜语句列＞:: = ｛＜语句＞｝
int Parser::statements()
{
    while (statement());
    return 1;
}
//＜语句＞    ::= ＜条件语句＞｜＜循环语句＞｜‘{’＜语句列＞‘}’｜＜有返回值函数调用语句＞;|＜无返回值函数调用语句＞;｜
//              ＜赋值语句＞;｜＜读语句＞;｜＜写语句＞;｜＜空＞;｜＜返回语句＞;
int Parser::statement()
{
    lexer.getsym();
    if (sym == IFTK)
        condition_statement();
    else if (sym == FORTK || sym == WHILETK)
        loop();
    else if (sym == LBRACE)//复合语句
    {
        statements();
        lexer.getsym();
        if (sym != RBRACE)
        {
            lexer.rollback(1);
            error("缺少}");
        }
    }
    else
    {
        if (sym == SCANFTK)
            read();
        else if (sym == PRINTFTK)
            write();
        else if (sym == SEMICN)
            lexer.rollback(1);
        else if (sym == RETURNTK)
            ret();
        else if (sym == IDEN)//赋值语句或函数调用语句
        {
            Kind k = UNKNOW;
            if (table.lookup(token, k))
            {
                switch (k)
                {
                case FUN://有返回值函数调用语句当作无返回值的处理
                case V_FUN:
                    call_void();
                    break;
                case VAR:
                case ARR:
                case PARA:
                    assign();
                    break;
                default:;
                }
            }
            else
            {
                error("未定义的标识符" + token, 1);
            }
        }
        else//不是语句，返回0
        {
            lexer.rollback(1);
            return 0;
        }
        //以分号结束
        lexer.getsym();
        if (sym != SEMICN)
        {
            lexer.rollback(1);
            error("缺少;");
        }
            
    }
    return 1;
}
//＜条件语句＞::=  if ‘(’＜条件＞‘)’＜语句＞［else＜语句＞］
//进入前先读1个单词
int Parser::condition_statement()
{
    lexer.getsym();
    if (sym != LPARENT)
        error("缺少(",1);
    condition();
    int index = middle.size()-1;//跳转语句的下标
    lexer.getsym();
    if (sym != RPARENT)
    {
        lexer.rollback(1);
        error("缺少)");
    }
    statement();
    lexer.getsym();
    if (sym != ELSETK)
    {
        middle.setLabel(index, middle.size());
        lexer.rollback(1);
    }
    else
    {
        middle.emit(J, "", "");//else后无条件跳转，标号等待回填
        int index2 = middle.size()-1;
        middle.setLabel(index, middle.size());
        statement();
        middle.setLabel(index2, middle.size());
    }
    return 1;
}
//＜条件＞::=  ＜表达式＞＜关系运算符＞＜表达式＞｜＜表达式＞ //表达式为0条件为假，否则为真
int Parser::condition()
{
    string exp1 = middle.alloc();
    expression(exp1);
    lexer.getsym();
    _operator op;
    switch (sym)
    {
    case EQL:
        op = JNE;
        break;
    case NEQ:
        op = JE;
        break;
    case LSS:
        op = JGE;
        break;
    case LEQ:
        op = JGT;
        break;
    case GRE:
        op = JLE;
        break; 
    case GEQ:
        op = JLT;
        break;
    default://只有一个表达式
        lexer.rollback(1);
        string exp2=middle.alloc();
        middle.emit(ASS_I, exp2, 0);
        middle.release(2);
        middle.emit(JE, exp1, exp2);//这里跳转的目标地址是不满足条件时跳转的标号，需要反填，下同
        return 1;
    }
    string exp2 = middle.alloc();
    expression(exp2);
    middle.release(2);
    middle.emit(op, exp1, exp2);
    return 1;
}
//＜循环语句＞   ::=  while ‘(’＜条件＞‘)’＜语句＞
//              | for‘(’＜标识符＞＝＜表达式＞;＜条件＞;＜标识符＞＝＜标识符＞(+|-)＜步长＞‘)’＜语句＞
int Parser::loop()
{
    if (sym == WHILETK)
    {
        lexer.getsym();
        if (sym != LPARENT)
            error("缺少(",1);
        int label = middle.size();
        condition();
        int index = middle.size() - 1;
        lexer.getsym();
        if (sym != RPARENT)
        {
            lexer.rollback(1);
            error("缺少)");
        }
        statement();
        middle.emit(J, "", "");
        middle.setLabel(middle.size() - 1, label);//往前跳不需要反填
        middle.setLabel(index, middle.size());
    }
    else//for
    {
        lexer.getsym();
        if (sym != LPARENT)
            error("缺少(", 1);
        lexer.getsym();
        assign();
        lexer.getsym();
        if (sym != SEMICN)
        {
            lexer.rollback(1);
            error("for循环语句中缺少第一个;");
        }

        int label1 = middle.size();
        condition();
        int index1 = middle.size()-1;
        middle.emit(J, "", "");
        int index2 = middle.size()-1;

        lexer.getsym();
        if (sym != SEMICN)
        {
            lexer.rollback(1);
            error("for循环语句中缺少第二个;");
        }

        lexer.getsym();
        if (sym != IDEN)
            error("for语句的第三部分=左边缺少标识符",1);
        lexer.getsym();
        if(sym!=ASSIGN)
            error("for语句的第三部分缺少=", 1);
        lexer.getsym();
        if(sym!=IDEN)
            error("for语句的第三部分=右边缺少标识符", 1);
        lexer.getsym();
        if (!(sym == PLUS || sym == MINU))
            error("for语句的步长前面缺少+或-",1);
        lexer.getsym();
        if (!(sym == INTCON))
            error("for语句缺少步长", 1);
        lexer.rollback(4);

        int label2 = middle.size();
        assign();
        middle.emit(J, "", "");
        middle.setLabel(middle.size() - 1, label1);

        lexer.getsym();
        if (sym != RPARENT)
        {
            lexer.rollback(1);
            error("缺少)");
        }

        middle.setLabel(index2, middle.size());
        statement();
        middle.emit(J, "", "");
        middle.setLabel(middle.size()-1,label2);
        middle.setLabel(index1, middle.size());
    }
    return 1;
}

//＜有返回值函数调用语句＞ :: = ＜标识符＞‘(’＜值参数表＞‘)’
int Parser::call(string &name)
{
    string fn = token;
    lexer.getsym();
    if (sym != LPARENT)
        error("缺少(",1);
    parameter_pass(fn);
    lexer.getsym();
    if (sym != RPARENT)
        error("缺少)",1);
    middle.emit(CALL, fn, "", name);
    return 1;
}
//＜无返回值函数调用语句＞ ::= ＜标识符＞‘(’＜值参数表＞‘)’
//进入前先读1个单词
int Parser::call_void()
{
    string fn = token;
    lexer.getsym();
    if (sym != LPARENT)
        error("缺少(", 1);
    parameter_pass(fn);
    lexer.getsym();
    if (sym != RPARENT)
        error("缺少)",1);
    middle.emit(CALL_VOID, fn, "","");
    return 1;
}
//＜值参数表＞::= ＜表达式＞{,＜表达式＞}｜＜空＞
//只检查参数个数，不进行类型一致性检查
int Parser::parameter_pass(const string& name)
{
    vector<Type> paras= table.getParas(name);
    int paras_num = 0;
    lexer.getsym();
    if (sym != RPARENT)//有参数
    {
        lexer.rollback(1);
        do
        {
            if(paras_num>=paras.size())
                break;
            string exp = middle.alloc();
            expression(exp);
            middle.release(1);
            if(paras[paras_num]==CHAR)
                middle.emit(PARAMETER_C, exp, "", "");
            else
                middle.emit(PARAMETER, exp, "", "");
            paras_num++;
            lexer.getsym();
        } while (sym == COMMA);

        if (sym == COMMA)//n==0
        {
            error("实参个数比形参多");
            //继续分析其余的参数
            do
            {
                string exp = middle.alloc();
                expression(exp);
                middle.release(1);
                lexer.getsym();
            } while (sym == COMMA);
        }
        else if (sym == RPARENT)
        {
            if (paras_num == paras.size())
            {
                lexer.rollback(1);
                return 1;
            }
            else//n>0
            {
                error("实参个数比形参少");//继续
            }
        }
        else
        {
            if (paras_num == paras.size())
                error("缺少)", 1);//****
            else //n>0
                error("缺少,", 1);
        }
    }
    else
    {
        if(paras_num!= paras.size())
            error("实参个数比形参少");//继续
    }
    lexer.rollback(1);//还在有问题
    return 1;
}
//＜赋值语句＞   ::=  ＜标识符＞＝＜表达式＞|＜标识符＞‘[’＜表达式＞‘]’=＜表达式＞
//进入前多读一个单词
int Parser::assign()
{
    if (sym != IDEN)
        error("赋值语句的左边不是一个标识符", 1);
    Kind k = UNKNOW;
    string name= token;//数组或变量的名字
    if(!table.lookup(name, k))
        error("未定义的标识符"+token);
    lexer.getsym();
    if (sym == ASSIGN)
    {
        //先检测
        if (!(k == VAR || k == PARA))
            error("赋值语句的左边不是一个变量");//可能是未定义的标识符，目前来看不可能是其他的了，下同
        string exp = middle.alloc();
        expression(exp);
        middle.release(1);
        middle.emit(ASS, exp, "", name);
    }
    else if(sym==LBRACK)
    {
        if (k != ARR)
            error("赋值语句的左边不是一个数组");

        string exp = middle.alloc();//下标
        expression(exp);

        lexer.getsym();
        if (sym != RBRACK)
        {
            lexer.rollback(1);
            error("缺少]");
        }
        lexer.getsym();
        if (sym != ASSIGN)
            error("赋值语句缺少=",1);
        string exp2 = middle.alloc();//值
        expression(exp2);
        middle.release(2);
        middle.emit(ASS_ARR, name, exp, exp2);//对数组赋值
    }
    else error("不是赋值语句",1);
    return 1;
}
//＜读语句＞::=  scanf ‘(’＜标识符＞{,＜标识符＞}‘)’
int Parser::read()
{
    lexer.getsym();
    if (sym != LPARENT)
        error("缺少(",1);
    do
    {
        lexer.getsym();
        if (sym != IDEN)
            error("缺少标识符",1);
        Kind k = UNKNOW;
        if (table.lookup(token, k))
        {
            if(k==VAR||k==PARA)
                middle.emit(SCANF, token, "", "");
            else error(token+"不是一个变量");
        }
        else error("未定义的标识符"+token);
        lexer.getsym();
    } while (sym == COMMA);
    if (sym != RPARENT)
    {
        lexer.rollback(1);
        error("缺少)");
    }
    return 1;
}
//＜写语句＞::=  printf ‘(’＜字符串＞,＜表达式＞‘)’| printf ‘(’＜字符串＞‘)’| printf ‘(’＜表达式＞‘)’
//进入前先读1个单词
//字符型表达式输出字符
int Parser::write()
{
    lexer.getsym();
    if (sym != LPARENT)
        error("缺少(", 1);
    lexer.getsym();
    if (sym == STRCON)
    {
        middle.emit(PRINTF_S, token,"","");
        lexer.getsym();
        if (sym == COMMA)
        {
            int print_char = 0;//是否输出字符，由表达式的类型决定
            string exp = middle.alloc();
            if (expression(exp) == CHAR)
                print_char = 1;
            middle.release(1);
            if (print_char)
                middle.emit(PRINTF_C, exp, "", "");
            else middle.emit(PRINTF, exp, "", "");
        }
        else
            lexer.rollback(1);
    }
    else//表达式
    {
        int print_char=0;
        lexer.rollback(1);
        string exp = middle.alloc();
        if(expression(exp)==CHAR)
            print_char=1;
        middle.release(1);
        if(print_char)
            middle.emit(PRINTF_C, exp, "", "");
        else middle.emit(PRINTF, exp, "", "");
    }
    lexer.getsym();
    if (sym != RPARENT)
    {
        lexer.rollback(1);
        error("缺少)");
    }
    return 1;
}
//＜返回语句＞   ::=  return[‘(’＜表达式＞‘)’]
//进入前先读1个单词
int Parser::ret()
{
    lexer.getsym();
    if (sym != LPARENT)
    {
        middle.emit(RETURN, "", "", "");
        lexer.rollback(1);
    }
    else
    {
        string exp = middle.alloc();
        expression(exp);
        middle.release(1);
        middle.emit(RETURN, exp, "", "");
        lexer.getsym();
        if (sym != RPARENT)
        {
            lexer.rollback(1);
            error("缺少)");
        }
    }
    return 1;
}



//＜表达式＞:: = ［＋｜－］＜项＞{ ＜加法运算符＞＜项＞ }
Type Parser::expression(string &name)
{
    Type t=NULL_TYPE;
    int flag = 0;
    lexer.getsym();
    if (sym == PLUS || sym == MINU)
    {
        t = INT;
        if (sym == MINU)
            flag = 1;
        lexer.getsym();
    }
    lexer.rollback(1);
    if(t==NULL_TYPE)
        t=term(name);
    else term(name);
    if (flag)
        middle.emit(MIN, name, "", name);
    _operator op;
    lexer.getsym();
    while (sym == PLUS || sym == MINU)
    {
        t = INT;
        op = sym == PLUS ? ADD : SUB;
        string ter=middle.alloc();
        term(ter);
        middle.release(1);
        middle.emit(op, name, ter, name);
        lexer.getsym();
    }
    lexer.rollback(1);
    return t;
}
//＜项＞::= ＜因子＞{ ＜乘法运算符＞＜因子＞ }
Type Parser::term(string &name)
{
    Type t;
    t=factor(name);
    _operator op;
    lexer.getsym();
    while (sym == MULT||sym==DIV)
    {
        t = INT;
        op = sym == MULT ? MUL : DIVE;
        string fac = middle.alloc();
        factor(fac);
        middle.release(1);
        middle.emit(op, name, fac, name);
        lexer.getsym();
    }
    lexer.rollback(1);
    return t;
}
//＜因子＞::= ＜标识符＞｜＜标识符＞‘[’＜表达式＞‘]’｜＜整数＞|＜字符＞｜＜有返回值函数调用语句＞|‘(’＜表达式＞‘)’
Type Parser::factor(string &name)
{
    Type t = NULL_TYPE;
    lexer.getsym();
    if (sym == IDEN)
    {
        Kind k = UNKNOW;
        table.lookup(token, k);
        t=table.getType(token);
        if (k == VAR || k == PARA)//变量
        {
            middle.emit(ASS, token, "",name);
            //name = token;
        }
        else if (k == CON)//常量
        {
            middle.emit(ASS_I, name, table.getValue(token));
        }
        else if (k == ARR)//数组
        {
            string arr_name = token;
            lexer.getsym();
            if (sym != LBRACK)
                error("缺少[",1);
            string index = middle.alloc();
            expression(index);
            lexer.getsym();
            if (sym != RBRACK)
            {
                lexer.rollback(1);
                error("缺少]");
            }
            middle.release(1);
            middle.emit(SUBSCRIPT, arr_name, index, name);
        }
        else if (k == FUN)//有返回值函数调用
        {
            call(name);
        }
        else if (k == V_FUN)
        {
            error(token + "无返回值的函数调用不能出现在表达式里");//继续分析
            call_void();
        }
        else
        {
            error("未定义的标识符"+token,1);//难以继续，所以就终止了
        }
        
    }
    else if (sym == CHARCON)//字符
    {
        t = CHAR;
        middle.emit(ASS_I,name,num);
    }
    else if (sym == LPARENT)//表达式
    {
        t=expression(name);
        lexer.getsym();
        if (sym != RPARENT)
            error("因子中的表达式缺少)",1);
            
    }
    else if (sym == ZERO || sym == INTCON || sym == PLUS || sym == MINU)//整数
    {
        t = INT;
        int value;
        lexer.rollback(1);
        if(!integer(value))
            error("表达式中出现无效的整数",1);
        middle.emit(ASS_I, name, value);
    }
    else
        error("无效的因子",1);
    return t;
}