/**
 * @file    calculator.cpp
 * @brief   简易计算器核心实现
 * @author  裴振羽
 * @date    2026-08-27
 * @version 1.0.0
 * 
 * @details 本文件实现了表达式解析器（Parser）和求值器（eval）。
 *          
 *          ## 核心算法：递归下降解析 + 表达式二叉树
 *          
 *          文法定义（优先级从低到高）：
 *          
 *          expression = term { ('+'|'-') term }
 *          term       = factor { ('*'|'/') factor }
 *          factor     = unary
 *          unary      = '-' unary | '+' unary | power
 *          power      = primary [ '^' unary ]
 *          primary    = number | func '(' args ')' | '(' expression ')'
 *          args       = expression [ ',' expression ]
 *          
 *          ## 函数支持
 *          | 函数名 | 参数 | 说明 |
 *          |--------|------|------|
 *          | sin    | x    | 正弦（角度制）|
 *          | cos    | x    | 余弦（角度制）|
 *          | tan    | x    | 正切（角度制）|
 *          | ln     | x    | 自然对数，x > 0 |
 *          | log    | x 或 (底,真数) | 常用对数或任意底数对数 |
 *          | exp    | x    | e^x |
 *          | sqrt   | x    | 平方根，x ≥ 0 |
 * 
 * @see     calculator.h 接口定义
 */

#include "calculator.h"
#include <cctype>
#include <cstdlib>
#include <cmath>

/**
 * @namespace   (匿名)
 * @brief       内部实现命名空间，对外部不可见
 * @details     包含 Parser 类、Node 结构体、eval 函数等核心实现细节。
 *              所有符号仅在当前编译单元可见，不暴露给外部。
 */
namespace {

/**
 * @const   PI
 * @brief   圆周率常量，用于角度到弧度的转换
 */
const double PI = 3.14159265358979323846;

/**
 * @enum    FuncType
 * @brief   支持的数学函数类型枚举
 */
enum FuncType {
    FUNC_SIN,   ///< 正弦函数
    FUNC_COS,   ///< 余弦函数
    FUNC_TAN,   ///< 正切函数
    FUNC_LN,    ///< 自然对数 ln(x)
    FUNC_LOG,   ///< 对数函数：log(x) 或 log(底, 真数)
    FUNC_EXP,   ///< 指数函数 e^x
    FUNC_SQRT   ///< 平方根函数 sqrt(x)
};

/**
 * @struct  Node
 * @brief   表达式二叉树的结点
 * 
 * @details 采用代数数据类型（ADT）风格设计，通过 type 字段区分三种类型：
 *          - NUMBER:  叶结点，存储数值
 *          - OPERATOR: 内部结点，存储运算符和左右子结点
 *          - FUNCTION: 内部结点，存储函数类型和参数结点
 * 
 * @note    使用裸指针管理子结点，通过 freeTree() 手动释放内存。
 *          这符合 RAII 原则的例外情况——本模块内部完全控制生命周期。
 */
struct Node {
    /**
     * @enum    Type
     * @brief   结点类型枚举
     */
    enum Type { NUMBER, OPERATOR, FUNCTION } type;
    
    double value;   ///< NUMBER 类型时有效，存储数值
    char op;        ///< OPERATOR 类型时有效，存储运算符字符（+ - * / ^）
    FuncType func;  ///< FUNCTION 类型时有效，存储函数类型
    Node* left;     ///< 左子结点：OPERATOR 的左操作数 / FUNCTION 的第一个参数
    Node* right;    ///< 右子结点：OPERATOR 的右操作数 / FUNCTION 的第二个参数（可能为空）

    /**
     * @brief   构造数值结点
     * @param   v   数值
     */
    explicit Node(double v)
        : type(NUMBER), value(v), op(0), func(FUNC_SIN), left(nullptr), right(nullptr) {}

    /**
     * @brief   构造运算符结点
     * @param   o   运算符（+ - * / ^）
     * @param   l   左操作数
     * @param   r   右操作数
     */
    Node(char o, Node* l, Node* r)
        : type(OPERATOR), value(0.0), op(o), func(FUNC_SIN), left(l), right(r) {}

    /**
     * @brief   构造函数结点
     * @param   f   函数类型
     * @param   a1  第一个参数
     * @param   a2  第二个参数（可选，仅 log 函数使用）
     */
    Node(FuncType f, Node* a1, Node* a2)
        : type(FUNCTION), value(0.0), op(0), func(f), left(a1), right(a2) {}
};

/**
 * @brief   递归释放表达式二叉树
 * @param   n   根结点指针
 * @note    后序遍历释放，确保不会访问已释放的内存
 */
void freeTree(Node* n) {
    if (!n) return;
    freeTree(n->left);
    freeTree(n->right);
    delete n;
}

/**
 * @brief   角度转弧度
 * @param   d   角度值（度）
 * @return  弧度值
 */
double deg2rad(double d) { return d * PI / 180.0; }

/**
 * @class   Parser
 * @brief   递归下降表达式解析器
 * 
 * @details 将输入的字符串表达式解析为抽象语法树（AST）。
 *          采用 LL(1) 递归下降解析策略，每个文法非终结符对应一个解析方法。
 * 
 * @note    解析器在遇到语法错误时会抛出 std::runtime_error。
 */
class Parser {
public:
    /**
     * @brief   构造解析器
     * @param   s   待解析的表达式字符串
     */
    explicit Parser(const std::string& s) : s_(s), pos_(0) {}

    /**
     * @brief   执行解析
     * @return  表达式二叉树的根结点指针
     * @throws  std::runtime_error 表达式语法错误
     */
    Node* parse() {
        Node* n = parseExpression();
        skipSpaces();
        if (pos_ != s_.size()) {
            freeTree(n);
            throw std::runtime_error("表达式含无法识别的字符");
        }
        return n;
    }

private:
    const std::string& s_;  ///< 待解析的表达式字符串（引用，避免拷贝）
    size_t pos_;            ///< 当前解析位置

    /**
     * @brief   跳过空白字符
     */
    void skipSpaces() {
        while (pos_ < s_.size() && std::isspace(static_cast<unsigned char>(s_[pos_])))
            ++pos_;
    }

    /**
     * @brief   查看当前字符（不消费）
     * @return  当前字符，若已到末尾则返回 '\0'
     */
    char peek() {
        skipSpaces();
        return pos_ < s_.size() ? s_[pos_] : '\0';
    }

    /**
     * @brief   获取当前字符（消费）
     * @return  当前字符，若已到末尾则返回 '\0'
     */
    char get() {
        skipSpaces();
        return pos_ < s_.size() ? s_[pos_++] : '\0';
    }

    /**
     * @brief   解析表达式：expression = term { ('+'|'-') term }
     * @return  表达式子树根结点
     */
    Node* parseExpression() {
        Node* left = parseTerm();
        for (;;) {
            char c = peek();
            if (c == '+' || c == '-') {
                get();
                Node* right = parseTerm();
                left = new Node(c, left, right);
            } else {
                return left;
            }
        }
    }

    /**
     * @brief   解析项：term = factor { ('*'|'/') factor }
     * @return  项子树根结点
     */
    Node* parseTerm() {
        Node* left = parseFactor();
        for (;;) {
            char c = peek();
            if (c == '*' || c == '/') {
                get();
                Node* right = parseFactor();
                left = new Node(c, left, right);
            } else {
                return left;
            }
        }
    }

    /**
     * @brief   解析因子（入口）：factor = unary
     * @return  因子子树根结点
     */
    Node* parseFactor() {
        return parseUnary();
    }

    /**
     * @brief   解析一元运算符：unary = '-' unary | '+' unary | power
     * @return  一元运算子树根结点
     * @note    一元负号优先级低于幂运算，即 -x^2 = -(x^2)
     */
    Node* parseUnary() {
        char c = peek();
        if (c == '-') {
            get();
            return new Node('-', new Node(0.0), parseUnary());
        }
        if (c == '+') {
            get();
            return parseUnary();
        }
        return parsePower();
    }

    /**
     * @brief   解析幂运算：power = primary [ '^' unary ]
     * @return  幂运算子树根结点
     * @note    幂运算是右结合的，即 2^3^2 = 2^(3^2)
     *          指数可以为负数或复合表达式
     */
    Node* parsePower() {
        Node* base = parsePrimary();
        if (peek() == '^') {
            get();
            Node* exp = parseUnary();
            return new Node('^', base, exp);
        }
        return base;
    }

    /**
     * @brief   解析基本表达式：primary = number | func '(' args ')' | '(' expression ')'
     * @return  基本表达式子树根结点
     */
    Node* parsePrimary() {
        char c = peek();
        if (c == '(') {
            get();
            Node* n = parseExpression();
            if (get() != ')') {
                freeTree(n);
                throw std::runtime_error("缺少右括号 ')'");
            }
            return n;
        }
        if (std::isdigit(static_cast<unsigned char>(c)) || c == '.') {
            return parseNumber();
        }
        if (std::isalpha(static_cast<unsigned char>(c))) {
            return parseFunction();
        }
        throw std::runtime_error("表达式格式错误");
    }

    /**
     * @brief   解析函数调用：func '(' args ')'
     * @return  函数调用子树根结点
     * @throws  std::runtime_error 函数名未知、括号缺失或参数数量错误
     */
    Node* parseFunction() {
        size_t start = pos_;
        while (pos_ < s_.size() && std::isalpha(static_cast<unsigned char>(s_[pos_])))
            ++pos_;
        std::string name = s_.substr(start, pos_ - start);
        for (auto& ch : name) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));

        skipSpaces();
        if (peek() != '(')
            throw std::runtime_error("函数 " + name + " 后需要括号 '('");
        get();

        Node* arg1 = parseExpression();
        Node* arg2 = nullptr;
        if (peek() == ',') {
            get();
            arg2 = parseExpression();
        }
        if (get() != ')') {
            freeTree(arg1);
            freeTree(arg2);
            throw std::runtime_error("缺少右括号 ')'");
        }
        return makeFunction(name, arg1, arg2);
    }

    /**
     * @brief   根据函数名创建对应的函数结点
     * @param   name    函数名
     * @param   arg1    第一个参数
     * @param   arg2    第二个参数（可为 nullptr）
     * @return  函数结点指针
     * @throws  std::runtime_error 未知函数名或参数数量错误
     */
    Node* makeFunction(const std::string& name, Node* arg1, Node* arg2) {
        FuncType f;
        if (name == "sin") f = FUNC_SIN;
        else if (name == "cos") f = FUNC_COS;
        else if (name == "tan") f = FUNC_TAN;
        else if (name == "ln") f = FUNC_LN;
        else if (name == "log") f = FUNC_LOG;
        else if (name == "exp") f = FUNC_EXP;
        else if (name == "sqrt") f = FUNC_SQRT;
        else {
            freeTree(arg1);
            freeTree(arg2);
            throw std::runtime_error("未知函数: " + name);
        }
        if (f != FUNC_LOG && arg2 != nullptr) {
            freeTree(arg1);
            freeTree(arg2);
            throw std::runtime_error("函数 " + name + " 只需一个参数");
        }
        return new Node(f, arg1, arg2);
    }

    /**
     * @brief   解析数字
     * @return  数值结点指针
     * @throws  std::runtime_error 缺少数字（如表达式以 '.' 结尾）
     */
    Node* parseNumber() {
        skipSpaces();
        size_t start = pos_;
        while (pos_ < s_.size() &&
               (std::isdigit(static_cast<unsigned char>(s_[pos_])) || s_[pos_] == '.'))
            ++pos_;
        if (pos_ == start)
            throw std::runtime_error("缺少数字");
        std::string num = s_.substr(start, pos_ - start);
        return new Node(std::strtod(num.c_str(), nullptr));
    }
};

/**
 * @brief   递归计算表达式二叉树的值
 * @param   n   根结点指针
 * @return  计算结果
 * @throws  std::runtime_error 除零、定义域错误、负数底数非整数次幂等
 * 
 * @note    后序遍历：先计算子结点，再计算当前结点
 */
double eval(const Node* n) {
    if (n->type == Node::NUMBER)
        return n->value;

    if (n->type == Node::OPERATOR) {
        double l = eval(n->left);
        double r = eval(n->right);
        switch (n->op) {
            case '+': return l + r;
            case '-': return l - r;
            case '*': return l * r;
            case '/':
                if (r == 0.0)
                    throw std::runtime_error("除数不能为零");
                return l / r;
            case '^':
                if (l < 0.0 && r != std::floor(r))
                    throw std::runtime_error("负数底数只能进行整数次幂");
                return std::pow(l, r);
            default:
                throw std::runtime_error("未知运算符");
        }
    }

    // FUNCTION 类型处理
    double arg = eval(n->left);
    double arg2 = n->right ? eval(n->right) : 0.0;
    switch (n->func) {
        case FUNC_SIN: return std::sin(deg2rad(arg));
        case FUNC_COS: return std::cos(deg2rad(arg));
        case FUNC_TAN: return std::tan(deg2rad(arg));
        case FUNC_LN:
            if (arg <= 0.0) throw std::runtime_error("ln 定义域要求 x > 0");
            return std::log(arg);
        case FUNC_LOG:
            if (n->right) {   // log(底, 真数)
                double base = arg, x = arg2;
                if (base <= 0.0 || base == 1.0) throw std::runtime_error("对数底数需 > 0 且 ≠ 1");
                if (x <= 0.0) throw std::runtime_error("对数真数需 > 0");
                return std::log(x) / std::log(base);
            }
            if (arg <= 0.0) throw std::runtime_error("log 定义域要求 x > 0");
            return std::log10(arg);
        case FUNC_EXP: return std::exp(arg);
        case FUNC_SQRT:
            if (arg < 0.0) throw std::runtime_error("sqrt 定义域要求 x ≥ 0");
            return std::sqrt(arg);
        default:
            throw std::runtime_error("未知函数");
    }
}

} // namespace

/**
 * @brief   Calculator::evaluate 的实现
 * 
 * @details 执行流程：
 *          1. 创建 Parser 实例解析表达式
 *          2. 得到抽象语法树（AST）的根结点
 *          3. 调用 eval 递归求值
 *          4. 释放 AST 内存
 *          5. 返回结果
 */
double Calculator::evaluate(const std::string& expression) {
    Parser parser(expression);
    Node* root = parser.parse();
    double result = eval(root);
    freeTree(root);
    return result;
}