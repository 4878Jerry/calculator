#pragma once
#include <string>
#include <stdexcept>

// 简易计算器核心：四则运算 + 括号 + 一元正负号 + 幂运算(^) + 常用数学函数。
// 内部使用「表达式二叉树(AST)」与递归下降解析,体现"数据结构(树) + 基础算法"。
//
// 支持的函数(可用任意大小写):
//   sin(x) cos(x) tan(x)  三角函数,参数为角度(度)
//   ln(x)   自然对数       log(x)  常用对数(log10),或 log(底, 真数)
//   exp(x)  e^x            sqrt(x) 平方根
// 幂运算 a^b 为右结合(2^3^2 = 2^(3^2)),优先级高于乘除。
class Calculator {
public:
    // 求值表达式;表达式非法时抛出 std::runtime_error
    double evaluate(const std::string& expression);
};
