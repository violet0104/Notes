#include "complex.h"

int main() {
    complex c1;
    cout << "c1: " << c1 << endl;
    complex c2(4, 5);
    cout << "c2: " << c2 << endl;

    complex c3(1, 2);
    cout << "c3: " << c3 << endl;

    c3 += c2;
    cout << "c3 += c2: " << c3 << endl;

    complex c4(3, 1);
    cout << "c4: " << c4 << endl;
    complex c5 = c4 - c3;
    cout << "c5 = c4 - c3: " << c5 << endl;

    return 0;
}