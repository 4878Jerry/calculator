#include <iostream>
#include <string>
#include "calculator.h"

#ifdef _WIN32
#include <windows.h>
#endif

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    std::cout << "===== 简易计算器 (阶段1: 四则运算 + 括号) =====\n";
    std::cout << "支持 + - * / ( ) 与一元负号;输入 exit 退出\n\n";

    Calculator calc;
    std::string line;
    for (;;) {
        std::cout << "> ";
        if (!std::getline(std::cin, line))
            break;
        if (line == "exit" || line == "quit")
            break;
        if (line.empty())
            continue;
        try {
            std::cout << "= " << calc.evaluate(line) << "\n\n";
        } catch (const std::exception& e) {
            std::cout << "错误: " << e.what() << "\n\n";
        }
    }
    return 0;
}
