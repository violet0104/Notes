#ifndef _MYSTRING_
#define _MYSTRING_

#include <iostream>
#include <cstring>

using namespace std;

class MyString;

ostream& operator << (ostream& os, const MyString& str);

// 类中有指针，必须写拷贝构造和拷贝赋值，不然会内存泄漏
// 因为c++默认是浅拷贝：只复制指针（内存地址），而不是指向的堆内存内容
class MyString
{
    public:
        MyString(const char* cstr = 0);
        MyString(const MyString& str);
        MyString& operator = (const MyString& str);
        ~MyString();
        char* get_c_str() const {return m_data;};

        friend ostream& operator << (ostream& os, const MyString& str);

    private:
        char* m_data;
};

// 构造函数
inline
MyString::MyString(const char* cstr) {
    if (cstr) {
        m_data = new char[strlen(cstr) + 1];
        strcpy(m_data, cstr);
    } else {
        m_data = new char[1];
        *m_data = '\0';
    }
}

// 拷贝构造函数
inline
MyString::MyString(const MyString& str) {
    m_data = new char[strlen(str.m_data) + 1];
    strcpy(m_data, str.m_data);
}

// 拷贝赋值函数
inline MyString& 
MyString::operator = (const MyString& str) {
    // 避免自我赋值
    if (this == &str) {
        return *this;
    }

    delete[] m_data;
    m_data = new char[strlen(str.m_data) + 1];
    strcpy(m_data, str.m_data);
    return *this;
}

// 析构函数：释放指针指向的内存
inline
MyString::~MyString() {
    cout << "调用析构函数" << endl;
    delete[] m_data;
}

// 操作符重载
inline ostream&
operator << (ostream& os, const MyString& str) {
    return os << str.m_data;
}


#endif