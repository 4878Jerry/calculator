#include "calculator.h"
#include <cctype>
#include <cstdlib>

namespace {

// 表达式二叉树结点
struct Node {
    enum Type { NUMBER, OPERATOR } type;
    double value;   // NUMBER 时有效
    char op;        // OPERATOR 时有效
    Node* left;
    Node* right;

    explicit Node(double v)
        : type(NUMBER), value(v), op(0), left(nullptr), right(nullptr) {}
    Node(char o, Node* l, Node* r)
        : type(OPERATOR), value(0.0), op(o), left(l), right(r) {}
};

void freeTree(Node* n) {
    if (!n) return;
    freeTree(n->left);
    freeTree(n->right);
    delete n;
}

// 递归下降解析器,文法：
//   expression = term { ('+'|'-') term }
//   term       = factor { ('*'|'/') factor }
//   factor     = number | '(' expression ')' | '-' factor | '+' factor
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
        if (c == '-') {              // 一元负号
            get();
            return new Node('-', new Node(0.0), parseFactor());
        }
        if (c == '+') {              // 一元正号
            get();
            return parseFactor();
        }
        if (std::isdigit(static_cast<unsigned char>(c)) || c == '.') {
            return parseNumber();
        }
        throw std::runtime_error("表达式格式错误");
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
        default:
            throw std::runtime_error("未知运算符");
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
