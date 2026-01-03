#include <iostream>
#include <bitset>

using namespace std;

void print() {

}

template <typename T, typename... Types>
void print (const T& firsrArg, const Types&... args) {
    cout << firsrArg << endl;
    cout << sizeof...(args) << endl;
    print(args...);
}

int main() {
    print(7.5, "hello", bitset<16>(377), 42);
}