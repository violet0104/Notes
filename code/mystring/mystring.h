#ifndef _MYSTRING_
#define _MYSTRING_

#include <iostream>

class MyString;

class MyString
{
    public:
        MyString(const char* cstr = 0);
        MyString(const MyString& str);
        MyString& operator = (const MyString& str);
        ~MyString();
        char* get_c_str() const {return m_data;};

    private:
        char* m_data;
};

// 构造函数


#endif