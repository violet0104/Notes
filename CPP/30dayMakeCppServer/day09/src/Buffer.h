#pragma once
#include <string>

class Buffer
{
private:
    std::string buf;
public:
    Buffer();
    ~Buffer();

    /// @brief 将数据追加到缓冲区
    void append(const char *_str, int _size);

    ssize_t size();

    /// @brief 返回缓冲区对应的C风格字符串指针
    const char *c_str();

    void clear();

    /// @brief 从标准输入（控制台）读取一行数据
    void getline();
};