#include <string>
using namespace std;
string m_to_string(int i)
{
    //string s;
    //int r=0;
    //char c;
    //do
    //{
    //  r=i%10;
    //  c='0'+r;
    //  s=s+c;
    //  i=i/10;
    //}while(i!=0);
    //return s;

    return to_string((long long)i);
}
