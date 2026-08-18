#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <sstream>
#include "calculator.h"

#ifdef _WIN32
#include <windows.h>
#endif

// 辅助：从文件读取所有历史记录到 vector
std::vector<std::string> loadHistory(const std::string& filename = "history.txt") {
    std::vector<std::string> lines;
    std::ifstream in(filename);
    if (!in.is_open()) return lines;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty()) lines.push_back(line);
    }
    return lines;
}

// 辅助：将 vector 写回文件（覆盖）
void saveHistory(const std::vector<std::string>& lines, const std::string& filename = "history.txt") {
    std::ofstream out(filename, std::ios::trunc);
    if (out.is_open()) {
        for (const auto& l : lines) {
            out << l << '\n';
        }
        out.close();
    }
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    std::cout << "===== 简易计算器 (扩展版) =====\n";
    std::cout << "支持 + - * / ( ) 与一元负号\n";
    std::cout << "命令: exit 退出 | history 查看历史 | clear 清空所有\n";
    std::cout << "      delete <编号> 删除单条 | search <关键字> 搜索 | stats 统计\n\n";

    Calculator calc;
    std::string line;

    for (;;) {
        std::cout << "> ";
        if (!std::getline(std::cin, line))
            break;

        if (line == "exit" || line == "quit")
            break;

        // ---------- 查看历史（带序号） ----------
        if (line == "history") {
            auto records = loadHistory();
            if (records.empty()) {
                std::cout << "暂无历史记录\n\n";
            } else {
                for (size_t i = 0; i < records.size(); ++i) {
                    std::cout << i + 1 << ". " << records[i] << '\n';
                }
                std::cout << '\n';
            }
            continue;
        }

        // ---------- 清空所有 ----------
        if (line == "clear" || line == "clear history") {
            std::ofstream out("history.txt", std::ios::trunc);
            if (out.is_open()) {
                out.close();
                std::cout << "所有历史记录已删除\n\n";
            } else {
                std::cout << "无法清空历史文件\n\n";
            }
            continue;
        }

        // ---------- 删除单条：delete <编号> ----------
        if (line.rfind("delete ", 0) == 0) {
            std::string numStr = line.substr(7);
            int index;
            try {
                index = std::stoi(numStr);
            } catch (...) {
                std::cout << "请输入有效的数字编号\n\n";
                continue;
            }
            auto records = loadHistory();
            if (index < 1 || index > static_cast<int>(records.size())) {
                std::cout << "编号无效，当前共有 " << records.size() << " 条记录\n\n";
                continue;
            }
            records.erase(records.begin() + (index - 1));
            saveHistory(records);
            std::cout << "已删除第 " << index << " 条记录\n\n";
            continue;
        }

        // ---------- 搜索：search <关键词> ----------
        if (line.rfind("search ", 0) == 0) {
            std::string keyword = line.substr(7);
            auto records = loadHistory();
            std::vector<std::string> found;
            for (const auto& rec : records) {
                if (rec.find(keyword) != std::string::npos) {
                    found.push_back(rec);
                }
            }
            if (found.empty()) {
                std::cout << "未找到包含 '" << keyword << "' 的记录\n\n";
            } else {
                std::cout << "找到 " << found.size() << " 条记录:\n";
                for (const auto& rec : found) {
                    std::cout << "  " << rec << '\n';
                }
                std::cout << '\n';
            }
            continue;
        }

        // ---------- 统计：stats ----------
        if (line == "stats") {
            auto records = loadHistory();
            std::cout << "历史记录总数: " << records.size() << '\n';
            if (!records.empty()) {
                double sum = 0.0;
                int count = 0;
                for (const auto& rec : records) {
                    size_t pos = rec.find(" = ");
                    if (pos != std::string::npos) {
                        std::string resultStr = rec.substr(pos + 3);
                        try {
                            double val = std::stod(resultStr);
                            sum += val;
                            count++;
                        } catch (...) {}
                    }
                }
                if (count > 0) {
                    std::cout << "平均结果: " << (sum / count) << '\n';
                }
            }
            std::cout << '\n';
            continue;
        }

        if (line.empty())
            continue;

        // ---------- 表达式计算 ----------
        try {
            double result = calc.evaluate(line);
            std::cout << "= " << result << "\n\n";

            // 追加保存历史
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