// named-reference.cpp
#include <iostream>
using namespace std;

// A class that contains a memory resource.
class MemoryBlock {
    // TODO: Add resources for the class here.
};

void g(const MemoryBlock&) {
    cout << "In g(const MemoryBlock&). " << endl;
}

void g(MemoryBlock&&) {
    cout << "In g(MemoryBlock&&). " << endl;
}

MemoryBlock&& f(MemoryBlock&& block) {
    g(block);       // block被视为左值
    // g(forward<MemoryBlock>(block));
    return move(block);
}

void F1() {
    g(f(MemoryBlock()));
}

void F2() {
    auto&& t = f(MemoryBlock());
    g(t);
}

int main() {
    F1();
    std::cout << "--- \n";
    F2();
}
