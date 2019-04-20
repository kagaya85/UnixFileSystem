#include <iostream>

using namespace std;

int main()
{
    unsigned char n = (unsigned char)255;
    unsigned char* p = &n;
    cout << "~(*p): " << (int)(unsigned char)~(*p) << endl;

    return 0;
}