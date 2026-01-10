#include <iostream>
#include <memory>

using namespace std;

int main() {
    // 创建
    shared_ptr<int> p1 = make_shared<int>(100);
    cout << "p1: " << *p1 << ", 引用计数: " << p1.use_count() << endl;
    // 输出：p1: 100, 引用计数: 1

    shared_ptr<int> p2 = p1;
    cout << "p2: " << *p2 << ", 引用计数: " << p1.use_count() << endl;
    // 输出：引用计数: 2（p1 和 p2 共享）

    shared_ptr<int> p3;
    p3 = p2;  // p3 加入共享
    cout << "引用计数: " << p1.use_count() << endl;
    // 输出：引用计数: 3

    p1.reset();  // p1 不再管这个对象
    cout << "p1 reset 后计数: " << p2.use_count() << endl;
    // 输出：2

    // 自动释放
    p2.reset();  // 计数=1
    p3.reset();  // 计数=0，内存在这里释放
}
