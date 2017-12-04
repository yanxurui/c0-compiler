#include <cctype>
#include <iostream>
#include <iomanip>
#include "Lexer.h"

string m_to_string(int i);

CLASS sym;//枚举型的单词类别码
string token;//标识符名字
int num;//无符号整数或字符的值

int line = 1;//记录当前行号
//err_no=0，表示缺少某个符号
//err_no=1，编译停止
void error(string info, int err_no = 0);

Lexer::Lexer(string file_name)
{
    //初始化
    reserver["const"] = CONSTTK;
    reserver["int"] = INTTK;
    reserver["char"] = CHARTK;
    reserver["void"] = VOIDTK;
    reserver["main"] = MAINTK;
    reserver["if"] = IFTK;
    reserver["else"] = ELSETK;
    reserver["while"] = WHILETK;
    reserver["for"] = FORTK;
    reserver["scanf"] = SCANFTK;
    reserver["printf"] = PRINTFTK;
    reserver["return"] = RETURNTK;
    reserver["+"] = PLUS;
    reserver["-"] = MINU;
    reserver["*"] = MULT;
    reserver["/"] = DIV;
    reserver["<"] = LSS;
    reserver["<="] = LEQ;
    reserver[">"] = GRE;
    reserver[">="] = GEQ;
    reserver["=="] = EQL;
    reserver["!="] = NEQ;
    reserver["="] = ASSIGN;
    reserver[";"] = SEMICN;
    reserver[","] = COMMA;
    reserver["("] = LPARENT;
    reserver[")"] = RPARENT;
    reserver["["] = LBRACK;
    reserver["]"] = RBRACK;
    reserver["{"] = LBRACE;
    reserver["}"] = RBRACE;
    ch = 0;
    r_char = 0;
    r_token = 0;
    
    fin.open(file_name);
    if (!fin)
    {
        cerr << "cannot open the file" << endl;
        exit(0);
    }
}

void Lexer::getch()
{
    if (r_char)
    {
        r_char = 0;
        if (ch == '\n')
            line++;
        return;
    }
    if((ch = fin.get()) != EOF)
    {
        if (!(ch >= 0 && ch <= 255))
        {
            error("illegal character ",2);
        }
        if (ch == '\n')//当前行结束
            line++;
        return;
    }
    cout << "program incomplete" << endl;
    fin.close();
    exit(0);
}
//回退一个字符
void Lexer::retract()
{
    r_char = 1;
    if (ch == '\n')//行号也要跟着改变
        line--;
}

void Lexer::rollback(int x)
{
    r_token += x+1;
    getsym();
}

void Lexer::getsym()
{
    if (r_token > 0)
    {
        rb.get(r_token, sym, token, num);
        r_token--;
    }
    else
    do
    {
        j = false;

        token = "";
        do
        {
            getch();//跳过空白符
        } while (isspace(ch));
        if (isLetter(ch))
        {
            do
            {
                token += tolower(ch);
                getch();
            } while (isLetter(ch) || isdigit(ch));

            auto it = reserver.find(token);
            if (it != reserver.end())
                sym = it->second;
            else sym = IDEN;
            retract();
        }
        else if (isdigit(ch))
        {
            if (ch == '0')
            {
                token = ch;//
                getch();
                if (!isdigit(ch))
                {
                    num = 0;
                    sym = ZERO;
                }
                else
                {
                    j = true;//以0开头的数字
                    error("以0开头的数字");
                }
                retract();
            }
            else
            {
                num = 0;
                do
                {
                    num = num * 10 + (ch - '0');
                    getch();
                } while (isdigit(ch));
                token = m_to_string(num);//
                sym = INTCON;
                retract();
            }
        }
        else if (ch == '\'')
        {
            getch();
            char temp = ch;
            token = ch;//
            if(!(isAdd(ch) || isMul(ch) || isLetter(ch) || isdigit(ch)))
            {
                error("字符常量不是合法的字符");//当做合法的处理
            }
            getch();
            if (ch == '\'')
            {
                num = temp;
                sym = CHARCON;
            }
            else
            {
                //先回退再报错，否则，报错的行号可能在下一行
                retract();
                error("字符常量缺少单引号");
                num = temp;
                sym = CHARCON;//自动补全
            }
        }
        else if (ch == '"')
        {
            token = "";
            getch();
            while (isChar(ch))
            {
                token += ch;
                getch();
            }
            if (ch == '"')
                sym = STRCON;
            else
            {
                retract();
                error("缺少右边的双引号或字符串中的字符不是32,33,35-126的ASCII字符");
                //可能是忘记双引号，但很有可能是字符串中间出现了回车或制表符
                //错误处理按第一种情况进行
                sym = STRCON;//自动补全
            }
        }
        else if (ch == '<')
        {
            getch();
            if (ch == '=')
            {
                token = "<=";
                sym = LEQ;
            }
            else
            {
                token = "<";
                sym = LSS;
                retract();
            }
        }
        else if (ch == '>')
        {
            getch();
            if (ch == '=')
            {
                token = ">=";
                sym = GEQ;
            }
            else
            {
                token = ">";
                sym = GRE;
                retract();
            }
        }
        else if (ch == '=')
        {
            getch();
            if (ch == '=')
            {
                token = "==";
                sym = EQL;
            }
            else
            {
                token = "=";
                sym = ASSIGN;
                retract();
            }
        }
        else if (ch == '!')
        {
            getch();
            if (ch != '=')
            {
                retract();
                error("！后面缺少=");
            }
            token = "!=";//当做!=处理
            sym = NEQ;
        }
        else
        {
            token = ch;
            if (ch == '+') sym = PLUS;
            else if (ch == '-') sym = MINU;
            else if (ch == '*') sym = MULT;
            else if (ch == '/') sym = DIV;
            else if (ch == ',') sym = COMMA;
            else if (ch == ';') sym = SEMICN;
            else if (ch == '(') sym = LPARENT;
            else if (ch == ')') sym = RPARENT;
            else if (ch == '[') sym = LBRACK;
            else if (ch == ']') sym = RBRACK;
            else if (ch == '{') sym = LBRACE;
            else if (ch == '}') sym = RBRACE;
            else
            {
                //无法识别的符号，跳过
                j = true;
            }
        }
        rb.push(sym, token, num);
    }while (j);
}

void Lexer::output()
{
    const string reserver_str[] =
    {
        "IDEN","INTCON","ZERO","CHARCON","STRCON",
        "CONSTTK","INTTK","CHARTK","VOIDTK",
        "MAINTK","IFTK","ELSETK","WHILETK","FORTK","SCANFTK","PRINTFTK","RETURNTK",
        "PLUS","MINU", "MULT","DIV","LSS","LEQ","GRE","GEQ","EQL","NEQ","ASSIGN",
        "SEMICN","COMMA","LPARENT","RPARENT","LBRACK","RBRACK","LBRACE","RBRACE",
        "NULL_CLASS"
    };
    cout << left;
    for (int i = 1;; i++)
    {
        getsym();
        cout <<setw(5)<< i << setw(10) << reserver_str[sym];
        switch (sym)
        {
        case INTCON:
        case ZERO:
            cout << num; break;
        case CHARCON:cout.put(num); break;
        default:cout << token;
        }
        cout << endl;
    }
    cout << right;
}


//内部类Rollback
void Lexer::Rollback::push(CLASS _sym, string _token, int _num)
{
    head=++head%max;
    S[head] = _sym;
    T[head] = _token;
    N[head] = _num;
}

void Lexer::Rollback::get(int i, CLASS &sym, string &token, int &num)
{
    int t = head;
    while (--i>0)
    {
        if (t == 0)
            t = max-1;
        else t--;
    }
    sym = S[t];
    token = T[t];
    num = N[t];
}
