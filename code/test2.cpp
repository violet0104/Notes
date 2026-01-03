#include <iostream>

template<class Key>
struct hash {};

template<>
struct hash<int> {
    size_t operator() (int x) const {return x;}
};


int main() {
    std::cout << ::hash<int>()(1000) << std::endl;
    return 0;
}