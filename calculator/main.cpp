#include <iostream>
#include <string>
#include <fstream>
#include <vector>           // 用于按行读取
#include "calculator.h"

#ifdef _WIN32
#include <windows.h>
#endif

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    std::cout << "===== 简易计算器 (阶段1: 四则运算 + 括号) =====\n";
    std::cout << "支持 + - * / ( ) 与一元负号\n";
    std::cout << "输入 exit 或 quit 退出，输入 history 查看历史记录\n\n";

    Calculator calc;
    std::string line;
    for (;;) {
        std::cout << "> ";
        if (!std::getline(std::cin, line))
            break;

        if (line == "exit" || line == "quit")
            break;

        if (line == "history") {
            // 查看历史记录
            std::ifstream histFile("history.txt");
            if (!histFile.is_open()) {
                std::cout << "暂无历史记录\n\n";
                continue;
            }
            std::string record;
            bool hasRecord = false;
            while (std::getline(histFile, record)) {
                if (!record.empty()) {
                    std::cout << record << '\n';
                    hasRecord = true;
                }
            }
            if (!hasRecord) {
                std::cout << "暂无历史记录";
            }
            std::cout << "\n";
            histFile.close();
            continue;
        }

        if (line.empty())
            continue;

        try {
            double result = calc.evaluate(line);
            std::cout << "= " << result << "\n\n";

            // 保存历史记录
            std::ofstream outFile("history.txt", std::ios::app);
            if (outFile.is_open()) {
                outFile << line << " = " << result << std::endl;
                outFile.close();
            }
        }
        catch (const std::exception& e) {
            std::cout << "错误: " << e.what() << "\n\n";
        }
    }
    return 0;
}