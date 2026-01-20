#include <iostream>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

int main()
{
    try {
        // 获取当前工作目录
        fs::path currentPath = fs::current_path();
        std::cout << "当前工作目录：" << currentPath.string() << std::endl;

        // 判断当前路径是否为目录
        if (fs::is_directory(currentPath)) {
            std::cout << "目录内容：" << std::endl;
            // 遍历目录中的所有条目
            for (const auto &entry : fs::directory_iterator(currentPath)) {
                std::cout << "  " << entry.path().filename().string() << std::endl;
            }
        } else {
            std::cout << "当前路径不是一个目录！" << std::endl;
        }
    } catch (const fs::filesystem_error &ex) {
        std::cerr << "Filesystem error: " << ex.what() << std::endl;
    } catch (const std::exception &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }
    return 0;
}
