#pragma once
#include <string>
#include <stdexcept>

// 简易计算器核心(阶段1)：四则运算 + 括号 + 一元正负号。
// 内部使用「表达式二叉树(AST)」与递归下降解析,体现"数据结构(树) + 基础算法"。
class Calculator {
public:
    // 求值表达式;表达式非法时抛出 std::runtime_error
    double evaluate(const std::string& expression);
};
