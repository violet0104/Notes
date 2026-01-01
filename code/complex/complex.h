#ifndef _COMPLEX_
#define _COMPLEX_

#include <cmath>
#include <iostream>

using namespace std;

class complex;

complex& _doapl(complex* ths, const complex& r);
ostream& operator << (ostream& os, const complex&);
complex operator - (const complex& x, const complex& y);

class complex
{
    public:
        complex(double r = 0, double i = 0) : re(r), im(i) {}
        // 操作符重载（成员函数）
        complex& operator += (const complex& r);
        double real() const {return re;};
        double imag() const {return im;};
        
        // 友元函数：函数可以直接取到re和im，即r.re, r.im
        friend complex& _doapl(complex* ths, const complex& r);
        friend ostream& operator << (ostream& os, const complex& x);

    private:
        double re, im;
};

inline complex&
_doapl(complex* ths, const complex& r) {
    ths->re += r.re;
    ths->im += r.im;
    return *ths;
}

inline complex&
complex::operator += (const complex& r) {
    return _doapl(this, r);
}

inline ostream&
operator << (ostream& os, const complex& x) {
    return os << '(' << x.re << ',' << x.im << ')';
}

// 操作符重载（非成员函数）
inline complex
operator - (const complex& x, const complex& y) {
    // 使用typename()创建一个typename类型的临时对象（返回对象而不是引用）
    return complex(x.real() - y.real(), x.imag() - y.imag());
}

#endif