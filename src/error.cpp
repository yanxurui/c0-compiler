#include <string>
#include <iostream>
using namespace std;
extern int line;
extern string token;//标识符名字
int errors = 0;
//err_no=0，表示缺少某个符号
//err_no=1，表示出现了不该出现的东西，编译停止
void error(string info,int err_no=0)
{
    if (err_no == 0)
    {
        errors++;
        cerr << "line" << line << "：";
        cerr << info << endl;
    }
    else if (err_no == 1)
    {
        cerr << "line" << line << "：";
        cerr << token + "不应该出现在这里，" + info << endl;
        exit(0);
    }
    else if(err_no==2)
    {
        cerr << "line" << line << "：";
        cerr << info << endl;
        exit(0);
    }
    else if(err_no==3)
    {
        cerr << info << endl;
        exit(0);
    }
}