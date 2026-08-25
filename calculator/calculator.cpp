#include "calculator.h"

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <string>
#include <unordered_map>

namespace {

constexpr double kPi = 3.14159265358979323846;

double deg2rad(double d) { return d * kPi / 180.0; }

// 可识别的函数类型
enum class FuncId { Sin, Cos, Tan, Ln, Log, Exp, Sqrt };

// 函数注册表(名称统一小写)。新增函数时只需在这里登记一行。
// 注: log 是唯一支持 2 个参数的函数: log(底, 真数);其余函数均只接受 1 个参数。
const std::unordered_map<std::string, FuncId>& functionTable() {
    static const std::unordered_map<std::string, FuncId> kTable = {
        {"sin",  FuncId::Sin},
        {"cos",  FuncId::Cos},
        {"tan",  FuncId::Tan},
        {"ln",   FuncId::Ln},
        {"log",  FuncId::Log},
        {"exp",  FuncId::Exp},
        {"sqrt", FuncId::Sqrt},
    };
    return kTable;
}

// 表达式二叉树结点。
// 使用 std::unique_ptr 管理子结点(RAII): 结点析构时自动递归释放整棵子树,
// 解析或求值过程中任何一步抛出异常都不会泄漏内存。
struct Node {
    enum class Kind { Number, Operator, Function } kind;

    double value = 0.0;             // Number 时有效
    char   op    = '\0';            // Operator 时有效
    FuncId func  = FuncId::Sin;     // Function 时有效

    std::unique_ptr<Node> left;     // Operator 左操作数 / Function 第一个参数
    std::unique_ptr<Node> right;    // Operator 右操作数 / Function 第二个参数(可为空)

    explicit Node(double v) : kind(Kind::Number), value(v) {}
    Node(char o, std::unique_ptr<Node> l, std::unique_ptr<Node> r)
        : kind(Kind::Operator), op(o), left(std::move(l)), right(std::move(r)) {}
    Node(FuncId f, std::unique_ptr<Node> a1, std::unique_ptr<Node> a2)
        : kind(Kind::Function), func(f), left(std::move(a1)), right(std::move(a2)) {}
};

// 递归下降解析器,文法(优先级从低到高):
//   expression = term { ('+'|'-') term }
//   term       = factor { ('*'|'/') factor }
//   factor     = unary
//   unary      = '-' unary | '+' unary | power
//   power      = primary [ '^' unary ]          // 右结合,指数可为负或嵌套
//   primary    = number | func '(' args ')' | '(' expression ')'
//   args       = expression [ ',' expression ]
class Parser {
public:
    explicit Parser(const std::string& s) : s_(s), pos_(0) {}

    std::unique_ptr<Node> parse() {
        auto n = parseExpression();
        skipSpaces();
        if (pos_ != s_.size())
            throw std::runtime_error("表达式含无法识别的字符");
        return n;
    }

private:
    const std::string& s_;
    size_t pos_;

    void skipSpaces() {
        while (pos_ < s_.size() && std::isspace(static_cast<unsigned char>(s_[pos_])))
            ++pos_;
    }
    char peek() {
        skipSpaces();
        return pos_ < s_.size() ? s_[pos_] : '\0';
    }
    char get() {
        skipSpaces();
        return pos_ < s_.size() ? s_[pos_++] : '\0';
    }

    std::unique_ptr<Node> parseExpression() {
        auto left = parseTerm();
        for (;;) {
            char c = peek();
            if (c != '+' && c != '-')
                return left;
            get();
            auto right = parseTerm();
            left = std::make_unique<Node>(c, std::move(left), std::move(right));
        }
    }

    std::unique_ptr<Node> parseTerm() {
        auto left = parseFactor();
        for (;;) {
            char c = peek();
            if (c != '*' && c != '/')
                return left;
            get();
            auto right = parseFactor();
            left = std::make_unique<Node>(c, std::move(left), std::move(right));
        }
    }

    std::unique_ptr<Node> parseFactor() {
        return parseUnary();
    }

    std::unique_ptr<Node> parseUnary() {
        char c = peek();
        if (c == '-') {                 // 一元负号(优先级低于 ^,即 -x^2 = -(x^2))
            get();
            return std::make_unique<Node>('-', std::make_unique<Node>(0.0), parseUnary());
        }
        if (c == '+') {                 // 一元正号
            get();
            return parseUnary();
        }
        return parsePower();
    }

    std::unique_ptr<Node> parsePower() {
        auto base = parsePrimary();
        if (peek() == '^') {
            get();
            auto exp = parseUnary();    // 指数可含负号或继续嵌套(右结合)
            return std::make_unique<Node>('^', std::move(base), std::move(exp));
        }
        return base;
    }

    std::unique_ptr<Node> parsePrimary() {
        char c = peek();
        if (c == '(') {
            get();
            auto n = parseExpression();
            if (get() != ')')
                throw std::runtime_error("缺少右括号 ')'");
            return n;
        }
        if (std::isdigit(static_cast<unsigned char>(c)) || c == '.')
            return parseNumber();
        if (std::isalpha(static_cast<unsigned char>(c)))
            return parseFunction();
        throw std::runtime_error("表达式格式错误");
    }

    std::unique_ptr<Node> parseNumber() {
        skipSpaces();
        size_t start = pos_;
        while (pos_ < s_.size() &&
               (std::isdigit(static_cast<unsigned char>(s_[pos_])) || s_[pos_] == '.'))
            ++pos_;
        if (pos_ == start)
            throw std::runtime_error("缺少数字");
        std::string num = s_.substr(start, pos_ - start);
        return std::make_unique<Node>(std::strtod(num.c_str(), nullptr));
    }

    std::unique_ptr<Node> parseFunction() {
        size_t start = pos_;
        while (pos_ < s_.size() && std::isalpha(static_cast<unsigned char>(s_[pos_])))
            ++pos_;
        std::string name = s_.substr(start, pos_ - start);
        for (auto& ch : name)
            ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));

        skipSpaces();
        if (peek() != '(')
            throw std::runtime_error("函数 " + name + " 后需要括号 '('");
        get();  // 吃掉 '('

        auto arg1 = parseExpression();
        std::unique_ptr<Node> arg2;
        if (peek() == ',') {
            get();
            arg2 = parseExpression();
        }
        if (get() != ')')
            throw std::runtime_error("缺少右括号 ')'");

        const auto& funcs = functionTable();
        const auto it = funcs.find(name);
        if (it == funcs.end())
            throw std::runtime_error("未知函数: " + name);
        if (it->second != FuncId::Log && arg2)
            throw std::runtime_error("函数 " + name + " 只需一个参数");
        return std::make_unique<Node>(it->second, std::move(arg1), std::move(arg2));
    }
};

double eval(const Node* n) {
    switch (n->kind) {
        case Node::Kind::Number:
            return n->value;

        case Node::Kind::Operator: {
            const double l = eval(n->left.get());
            const double r = eval(n->right.get());
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

        case Node::Kind::Function: {
            const double a1 = eval(n->left.get());
            const double a2 = n->right ? eval(n->right.get()) : 0.0;
            switch (n->func) {
                case FuncId::Sin: return std::sin(deg2rad(a1));
                case FuncId::Cos: return std::cos(deg2rad(a1));
                case FuncId::Tan: return std::tan(deg2rad(a1));
                case FuncId::Ln:
                    if (a1 <= 0.0)
                        throw std::runtime_error("ln 定义域要求 x > 0");
                    return std::log(a1);
                case FuncId::Log:
                    if (n->right) {     // log(底, 真数)
                        const double base = a1, x = a2;
                        if (base <= 0.0 || base == 1.0)
                            throw std::runtime_error("对数底数需 > 0 且 ≠ 1");
                        if (x <= 0.0)
                            throw std::runtime_error("对数真数需 > 0");
                        return std::log(x) / std::log(base);
                    }
                    if (a1 <= 0.0)
                        throw std::runtime_error("log 定义域要求 x > 0");
                    return std::log10(a1);
                case FuncId::Exp: return std::exp(a1);
                case FuncId::Sqrt:
                    if (a1 < 0.0)
                        throw std::runtime_error("sqrt 定义域要求 x ≥ 0");
                    return std::sqrt(a1);
                default:
                    throw std::runtime_error("未知函数");
            }
        }
    }
    throw std::runtime_error("未知节点类型");
}

} // namespace

double Calculator::evaluate(const std::string& expression) {
    Parser parser(expression);
    auto root = parser.parse();
    return eval(root.get());
}
