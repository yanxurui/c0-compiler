#include "Table.h"

string m_to_string(int i);

Table::Table()
{
    table t;
    tables.push_back(t);
    vector<string> v;
    ident_lists.push_back(v);
    cur=0;
}

int Table::insert(string name, Kind k, Type t, int val)
{
    table_value tv;
    tv.kind = k;
    if (k == VAR || k == CON || k == ARR||k==PARA)
    {
        if (lookup_local(name) == 1)
            return 0;
        if (k == CON || k == ARR)
            tv.value = val;//常量值或数组长度
        tv.type = t;
        
        //补充地址信息
        if (cur == 0 && (k == VAR||k==ARR))//全局变量数组
        {
            tv.addr = "-1";
        }
        else if (k == PARA)
        {
            int i = ident_lists[cur].size();//第i+1个参数
            if (i < 4)//前4个参数
            {
                tv.addr = "a" + m_to_string(i);
            }
            //else//由generate补充
        }

        tables[cur][name] = tv;
        ident_lists[cur].push_back(name);
    }
    else//函数
    {
        if (lookup_global(name) == 1)
            return 0;
        create_table();//新建符号表
        tv.value = cur;//指向私有符号表
        if (k == FUN)
            tv.type = t;
        tables[0][name] = tv;//函数名字插入全局符号表中
        ident_lists[0].push_back(name);
    }

    return 1;
}

int Table::lookup(string name,Kind &k)
{
    auto it = tables[cur].find(name);
    if (it != tables[cur].end())
    {
        if (k == UNKNOW)
        {
            k = it->second.kind;
            return 1;
        }
        else
        {
            if (k == it->second.kind)
            {
                return 1;
            }
        }

    }

    it = tables[0].find(name);
    if (it != tables[0].end())
    {
        if (k == UNKNOW)
        {
            k = it->second.kind;
            return 1;
        }
        else
        {
            if (k == it->second.kind)
            {
                return 1;
            }
        }
    }
    return 0;
}

void Table::create_table()
{
    table t;
    tables.push_back(t);
    vector<string> ident_list;
    ident_lists.push_back(ident_list);
    cur++;
}

int Table::lookup_local(string name)
{
    auto it = tables[cur].find(name);
    if (it != tables[cur].end())
        return 1;
    else return 0;
}

int Table::lookup_global(string name)
{
    auto it = tables[0].find(name);
    if (it != tables[0].end())
        return 1;
    else return 0;
}


int Table::getValue(string name)
{
    auto it = tables[cur].find(name);
    if (it != tables[cur].end())
    {
        return it->second.value;
    }
    it = tables[0].find(name);
    if (it != tables[0].end())
    {
        return it->second.value;
    }
    return 0;//error
}

vector<Type> Table::getParas(const string &name)
{
    int index = tables[0][name].value;
    table &t = tables[index];//该函数的符号表
    vector<string>& ident_list = ident_lists[index];//符号表的名字列表
    vector<Type> v;
    for (int i = 0; i < ident_list.size();i++)
    {
        string ident = ident_list[i];
        if (t[ident].kind == PARA)
            v.push_back(t[ident].type);
        else break;
    }
    return v;
}

Type Table::getType(const string &name)
{
    Type t= NULL_TYPE;
    auto it = tables[cur].find(name);
    if (it != tables[cur].end())
    {
        t = it->second.type;
    }

    it = tables[0].find(name);
    if (it != tables[0].end())
    {
        t = it->second.type;
    }
    return t;
}

void Table::setAddr(string &name,string addr)
{
    tables[cur][name].addr = addr;
}

string Table::getAddr(string &name)
{
    if (lookup_local(name) == 1)
    {
        return tables[cur][name].addr;
    }
    if (lookup_global(name) == 1)
        return tables[0][name].addr;
    return "-2";//找不到，临时变量
}

void Table::setCursor(int i)
{
    cur = i;
}
