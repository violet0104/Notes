#include "mystring.h"

int main() {
    MyString s1;
    cout << "s1: " << s1 << endl;

    MyString s2("Hello");
    cout << "s2: " << s2 << endl;

    // 拷贝赋值
    MyString s3 = s2;
    cout << "s3: " << s3 << endl;

    // 拷贝构造
    MyString s4(s2);
    cout << "s4: " << s4 << endl;
}