#include <iostream>

template <class T>
void process(T &&t) {
    std::cout << t << " is " << "rvalue\n";
}

template <class T>
void process(T &t) {
    std::cout << t << " is " << "lvalue\n";
}

template <typename T>
// t是万能引用
void wrapper(T &&t) {
    process(std::forward<T>(t));
}

template <typename T>
void wrapper_common(T &&t) {
    process(t);
}

int main() {
    // 测试右值引用
    wrapper(1);                     // rvalue

    // 测试左值引用                   // lvalue
    int i = 2;
    wrapper(i);

    // 测试完美转发将亡值
    // move()将左值i转成右值
    wrapper(std::move(i));          // rvalue

    // 测试不用完美转发
    int j = 3;
    wrapper_common(std::move(i));   // lvalue
    return 0;
}
