/**
 * @file    calculator.h
 * @brief   简易计算器核心接口定义
 * @author  裴振羽
 * @date    2026-08-27
 * @version 1.0.0
 * 
 * @details 本文件定义了 Calculator 类，该类是计算器核心功能的唯一对外接口。
 *          内部采用表达式二叉树（AST）和递归下降解析器实现表达式求值。
 *          
 *          支持的运算：
 *          - 四则运算：+ - * /
 *          - 括号：()
 *          - 幂运算：^（右结合，优先级高于乘除）
 *          - 一元正负号：+ -
 *          - 数学函数：sin, cos, tan, ln, log, exp, sqrt
 * 
 *          函数调用支持大小写不敏感，如 sin(30) 和 SIN(30) 均可。
 */

#pragma once
<<<<<<< HEAD
=======

#include <string>
>>>>>>> e165bd5 (docs: 为第三次作业添加完整的 Doxygen 风格代码注释)
#include <stdexcept>
#include <string>

/**
 * @class   Calculator
 * @brief   科学计算器核心类
 * 
 * @details 该类封装了表达式的语法分析、语法树构建和求值全过程。
 *          使用递归下降解析器将输入的字符串表达式解析为抽象语法树（AST），
 *          然后递归遍历 AST 计算出最终结果。
 * 
 * @note    所有异常通过 std::runtime_error 抛出，调用者需捕获处理。
 * 
 * @example 
 * @code
 *   Calculator calc;
 *   double result = calc.evaluate("sin(30) + 2^3");
 *   std::cout << result;  // 输出: 8.5
 * @endcode
 */
class Calculator {
public:
    /**
     * @brief   计算表达式的值
     * 
     * @param   expression 待求值的数学表达式，格式为字符串。
     *                      支持的语法规则：
     *                      - 数字：整数或浮点数，如 123, 3.14
     *                      - 四则运算：+ - * /，支持括号改变优先级
     *                      - 幂运算：^，右结合，如 2^3^2 = 2^(3^2)
     *                      - 一元正负号：+3, -5
     *                      - 函数调用：函数名(参数)，如 sin(30)
     *                      - 多参数函数：log(底数, 真数)
     * 
     * @return  表达式的计算结果，类型为 double
     * 
     * @throws  std::runtime_error 在以下情况会抛出异常：
     *          - 表达式语法错误（如括号不匹配、未知字符）
     *          - 除数为零
     *          - 函数定义域错误（如 ln(0)、sqrt(-1)）
     *          - 未知函数名
     *          - 负数底数进行非整数次幂运算
     *          - 对数底数 ≤ 0 或 = 1
     * 
     * @note    三角函数 sin/cos/tan 的参数采用角度制（度），
     *          如 sin(30) 返回 0.5
     * 
     * @see     https://en.wikipedia.org/wiki/Recursive_descent_parser
     */
    double evaluate(const std::string& expression);
};