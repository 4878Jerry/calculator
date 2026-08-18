#include "calculator.h"
#include <cctype>
#include <cstdlib>
#include <cmath>

namespace {

const double PI = 3.14159265358979323846;

// 可识别的函数类型
enum FuncType {
    FUNC_SIN, FUNC_COS, FUNC_TAN,
    FUNC_LN,  FUNC_LOG, FUNC_EXP, FUNC_SQRT
};

// 表达式二叉树结点
struct Node {
    enum Type { NUMBER, OPERATOR, FUNCTION } type;
    double value;   // NUMBER 时有效
    char op;        // OPERATOR 时有效
    FuncType func;  // FUNCTION 时有效
    Node* left;     // OPERATOR 左操作数 / FUNCTION 第一个参数
    Node* right;    // OPERATOR 右操作数 / FUNCTION 第二个参数(可能为空)

    explicit Node(double v)
        : type(NUMBER), value(v), op(0), func(FUNC_SIN), left(nullptr), right(nullptr) {}
    Node(char o, Node* l, Node* r)
        : type(OPERATOR), value(0.0), op(o), func(FUNC_SIN), left(l), right(r) {}
    Node(FuncType f, Node* a1, Node* a2)
        : type(FUNCTION), value(0.0), op(0), func(f), left(a1), right(a2) {}
};

void freeTree(Node* n) {
    if (!n) return;
    freeTree(n->left);
    freeTree(n->right);
    delete n;
}

double deg2rad(double d) { return d * PI / 180.0; }

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

    Node* parseFactor() {
        return parseUnary();
    }

    Node* parseUnary() {
        char c = peek();
        if (c == '-') {              // 一元负号(优先级低于 ^,即 -x^2 = -(x^2))
            get();
            return new Node('-', new Node(0.0), parseUnary());
        }
        if (c == '+') {              // 一元正号
            get();
            return parseUnary();
        }
        return parsePower();
    }

    Node* parsePower() {
        Node* base = parsePrimary();
        if (peek() == '^') {
            get();
            Node* exp = parseUnary();   // 指数可含负号或继续嵌套(右结合)
            return new Node('^', base, exp);
        }
        return base;
    }

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

    Node* parseFunction() {
        size_t start = pos_;
        while (pos_ < s_.size() && std::isalpha(static_cast<unsigned char>(s_[pos_])))
            ++pos_;
        std::string name = s_.substr(start, pos_ - start);
        for (auto& ch : name) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));

        skipSpaces();
        if (peek() != '(')
            throw std::runtime_error("函数 " + name + " 后需要括号 '('");
        get();  // 吃掉 '('

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

    Node* makeFunction(const std::string& name, Node* arg1, Node* arg2) {
        FuncType f;
        if (name == "sin") f = FUNC_SIN;
        else if (name == "cos") f = FUNC_COS;
        else if (name == "tan") f = FUNC_TAN;
        else if (name == "ln") f = FUNC_LN;
        else if (name == "log") f = FUNC_LOG;   // 1 参=log10,2 参=log(底,真数)
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

    // FUNCTION
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

double Calculator::evaluate(const std::string& expression) {
    Parser parser(expression);
    Node* root = parser.parse();
    double result = eval(root);
    freeTree(root);
    return result;
}
