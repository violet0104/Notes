#include <iostream>
#include <functional>

class Task
{
    using task_t = std::function<void()>;
public:
    template<typename F, typename... Args>
    Task(F&& f, Args&& ...args) : func(std::bind(std::forward<F>(f), std::forward<Args>(args)...)) {}

    void execute() {
        if (func) {
            func();
        }
    }
private:
    task_t func;
};

void add(int x, int y) {
    std::cout << x + y << std::endl;
}

int main() {
    Task task(add, 1, 2);
    task.execute();
    return 0;
}