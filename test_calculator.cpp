// ============================================================================
// 简易计算器 —— 单元测试
//
// 被测对象: Calculator::evaluate (calculator.h / calculator.cpp)
// 说明: main.cpp (Win32 GUI) 与 first.cpp (菜单版) 属于界面/入口层,
//       难以进行自动化单元测试, 本测试聚焦于可复用的表达式求值核心。
//
// 编译运行 (MinGW / g++):
//     g++ -std=c++11 -Wall -Wextra -o test_calculator.exe test_calculator.cpp calculator.cpp
//     test_calculator.exe
//
// 编译运行 (MSVC, /utf-8 保证源码中文字符串字面量正确):
//     cl /EHsc /W4 /utf-8 test_calculator.cpp calculator.cpp
//     test_calculator.exe
//
// 也可以直接双击 build_and_run.bat 一键编译并运行。
// ============================================================================

#include "calculator.h"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// ---------------- 极简自包含测试框架(无第三方依赖) ----------------
namespace {

struct TestEntry {
    const char* name;
    void (*fn)();
};

std::vector<TestEntry>& registry() {
    static std::vector<TestEntry> reg;
    return reg;
}

struct Registrar {
    Registrar(const char* name, void (*fn)()) { registry().push_back(TestEntry{name, fn}); }
};

int g_checks = 0;     // 全部检查点个数
int g_failures = 0;   // 全部失败个数
int g_testFails = 0;  // 当前用例失败个数

void reportFail(const char* file, int line, const std::string& msg) {
    ++g_failures;
    ++g_testFails;
    std::cout << "      [FAIL] " << file << ":" << line << " : " << msg << "\n";
}

// 带绝对/相对容差的浮点比较
bool nearlyEqual(double a, double b, double relTol, double absTol) {
    return std::fabs(a - b) <= std::fmax(absTol, relTol * std::fmax(std::fabs(a), std::fabs(b)));
}

} // namespace

#define TEST(name) \
    static void test_##name(); \
    static Registrar reg_##name(#name, &test_##name); \
    static void test_##name()

#define CHECK(cond) \
    do { \
        ++g_checks; \
        if (!(cond)) reportFail(__FILE__, __LINE__, "CHECK(" #cond ")"); \
    } while (0)

#define CHECK_NEAR(actual, expected) \
    do { \
        double a_ = (actual), e_ = (expected); \
        ++g_checks; \
        if (!nearlyEqual(a_, e_, 1e-9, 1e-12)) { \
            reportFail(__FILE__, __LINE__, "CHECK_NEAR(" #actual ", " #expected ")"); \
            std::cout << "            actual = " << a_ << ", expected = " << e_ << "\n"; \
        } \
    } while (0)

#define CHECK_THROWS(expr) \
    do { \
        bool threw_ = false; \
        ++g_checks; \
        try { (void)(expr); } \
        catch (const std::exception&) { threw_ = true; } \
        if (!threw_) reportFail(__FILE__, __LINE__, "CHECK_THROWS(" #expr ") 未抛出异常"); \
    } while (0)

// ---------------- 辅助函数 ----------------

static Calculator g_calc;

static double calc(const std::string& expr) {
    return g_calc.evaluate(expr);
}

// 期望抛出异常, 且异常消息包含指定子串
static void expectThrowContains(const std::string& expr, const std::string& substr) {
    ++g_checks;
    try {
        (void)calc(expr);
        reportFail(__FILE__, __LINE__, "expectThrowContains(\"" + expr + "\") 期望异常但未抛出");
    } catch (const std::exception& e) {
        std::string what = e.what();
        if (what.find(substr) == std::string::npos) {
            reportFail(__FILE__, __LINE__,
                       "异常消息 [" + what + "] 不含期望子串 [" + substr + "] (表达式: " + expr + ")");
        }
    }
}

// ============================================================================
// 1. 基本四则运算
// ============================================================================

TEST(addition_basic) {
    CHECK_NEAR(calc("1+2"), 3);
    CHECK_NEAR(calc("0.5+0.25"), 0.75);
    CHECK_NEAR(calc("123+456"), 579);
    CHECK_NEAR(calc("1+2+3+4+5"), 15);
}

TEST(subtraction_basic) {
    CHECK_NEAR(calc("5-3"), 2);
    CHECK_NEAR(calc("3-5"), -2);
    CHECK_NEAR(calc("1-1-1-1"), -2);  // 左结合
    CHECK_NEAR(calc("10-2-3"), 5);
}

TEST(multiplication_basic) {
    CHECK_NEAR(calc("2*3"), 6);
    CHECK_NEAR(calc("2.5*4"), 10);
    CHECK_NEAR(calc("2*3*4"), 24);
}

TEST(division_basic) {
    CHECK_NEAR(calc("7/2"), 3.5);
    CHECK_NEAR(calc("1/3"), 1.0 / 3.0);
    CHECK_NEAR(calc("100/8"), 12.5);
    CHECK_NEAR(calc("10/2/5"), 1);  // 左结合: (10/2)/5
}

TEST(operator_precedence) {
    CHECK_NEAR(calc("2+3*4"), 14);          // 先乘除
    CHECK_NEAR(calc("2*3+4"), 10);
    CHECK_NEAR(calc("2+3*4-6/2"), 11);
    CHECK_NEAR(calc("10-2*3+4/2"), 6);
    CHECK_NEAR(calc("2*3+4*5"), 26);
}

TEST(parentheses_grouping) {
    CHECK_NEAR(calc("(1)"), 1);
    CHECK_NEAR(calc("((((2))))"), 2);
    CHECK_NEAR(calc("(2+3)*4"), 20);        // 括号改变优先级
    CHECK_NEAR(calc("2*(3+4)"), 14);
    CHECK_NEAR(calc("(1+2)*(3+4)"), 21);
    CHECK_NEAR(calc("(2+3)*4-1"), 19);
    CHECK_NEAR(calc("(1+2*3)/(4-1)"), 7.0 / 3.0);
}

// ============================================================================
// 2. 一元正负号
// ============================================================================

TEST(unary_minus) {
    CHECK_NEAR(calc("-5"), -5);
    CHECK_NEAR(calc("--5"), 5);      // 负负得正
    CHECK_NEAR(calc("---5"), -5);
    CHECK_NEAR(calc("-3+4"), 1);
    CHECK_NEAR(calc("5*-3"), -15);
    CHECK_NEAR(calc("2--3"), 5);
    CHECK_NEAR(calc(" - 5 "), -5);   // 负号与数字之间允许空格
    // 一元负号优先级低于 '^': -2^2 = -(2^2)
    CHECK_NEAR(calc("-2^2"), -4);
    CHECK_NEAR(calc("(-2)^2"), 4);
    CHECK_NEAR(calc("2^-2"), 0.25);  // 指数可为负
    CHECK_NEAR(calc("-2^0.5"), -std::sqrt(2.0));
}

TEST(unary_plus) {
    CHECK_NEAR(calc("+5"), 5);
    CHECK_NEAR(calc("1++2"), 3);
    CHECK_NEAR(calc("1+-2"), -1);
    CHECK_NEAR(calc("+3*+4"), 12);
}

// ============================================================================
// 3. 幂运算 '^'
// ============================================================================

TEST(power_basic) {
    CHECK_NEAR(calc("2^10"), 1024);
    CHECK_NEAR(calc("3^3"), 27);
    CHECK_NEAR(calc("2^0"), 1);
    CHECK_NEAR(calc("2^0.5"), std::sqrt(2.0));
    CHECK_NEAR(calc("9^0.5"), 3);
    CHECK_NEAR(calc("(-2)^3"), -8);  // 负数底数整数次幂合法
    CHECK_NEAR(calc("(-2)^2"), 4);
}

TEST(power_right_assoc) {
    // 幂是右结合: 2^3^2 = 2^(3^2) = 2^9 = 512
    CHECK_NEAR(calc("2^3^2"), 512);
    CHECK_NEAR(calc("2^2^3"), 256);  // 2^(2^3)
}

TEST(power_precedence) {
    CHECK_NEAR(calc("2*3^2"), 18);   // 幂优先于乘
    CHECK_NEAR(calc("3^2*2"), 18);
    CHECK_NEAR(calc("2^3+1"), 9);    // 幂优先于加
}

// ============================================================================
// 4. 数学函数
// ============================================================================

TEST(trig_functions) {
    CHECK_NEAR(calc("sin(0)"), 0);
    CHECK_NEAR(calc("sin(30)"), 0.5);       // 角度制
    CHECK_NEAR(calc("sin(90)"), 1);
    CHECK_NEAR(calc("cos(0)"), 1);
    CHECK_NEAR(calc("cos(60)"), 0.5);
    CHECK_NEAR(calc("cos(180)"), -1);
    CHECK_NEAR(calc("tan(0)"), 0);
    CHECK_NEAR(calc("tan(45)"), 1);
    CHECK_NEAR(calc("sin(45)"), std::sqrt(2.0) / 2.0);
}

TEST(exp_ln_log) {
    CHECK_NEAR(calc("exp(0)"), 1);
    CHECK_NEAR(calc("exp(1)"), std::exp(1.0));
    CHECK_NEAR(calc("ln(1)"), 0);
    CHECK_NEAR(calc("ln(exp(1))"), 1);
    CHECK_NEAR(calc("ln(2)"), std::log(2.0));
    CHECK_NEAR(calc("log(1)"), 0);          // 单参数 = log10
    CHECK_NEAR(calc("log(10)"), 1);
    CHECK_NEAR(calc("log(100)"), 2);
    CHECK_NEAR(calc("log(1000)"), 3);
    CHECK_NEAR(calc("log(2)"), std::log10(2.0));
    CHECK_NEAR(calc("log(2,8)"), 3);        // 双参数 = 换底公式
    CHECK_NEAR(calc("log(2,1024)"), 10);
    CHECK_NEAR(calc("log(10,100)"), 2);
    CHECK_NEAR(calc("log(3,27)"), 3);
}

TEST(sqrt_function) {
    CHECK_NEAR(calc("sqrt(0)"), 0);
    CHECK_NEAR(calc("sqrt(16)"), 4);
    CHECK_NEAR(calc("sqrt(2)"), std::sqrt(2.0));
    CHECK_NEAR(calc("sqrt(2)^2"), 2);       // 函数与运算符组合
    CHECK_NEAR(calc("sqrt(sqrt(16))"), 2);  // 函数嵌套
}

TEST(function_case_insensitive) {
    CHECK_NEAR(calc("SIN(30)"), 0.5);
    CHECK_NEAR(calc("Sin(30)"), 0.5);
    CHECK_NEAR(calc("SQRT(16)"), 4);
    CHECK_NEAR(calc("Ln(1)"), 0);
    CHECK_NEAR(calc("LOG(100)"), 2);
    CHECK_NEAR(calc("Exp(0)"), 1);
}

TEST(function_in_expression) {
    CHECK_NEAR(calc("sin(30)+cos(60)"), 1);
    CHECK_NEAR(calc("2*sqrt(9)"), 6);
    CHECK_NEAR(calc("sqrt(9)+sqrt(16)"), 7);
    CHECK_NEAR(calc("2*sin(30)"), 1);
    CHECK_NEAR(calc("log(2,8)+ln(1)"), 3);
    CHECK_NEAR(calc("sqrt(3^2+4^2)"), 5);   // 勾股定理: 参数为复合表达式
    CHECK_NEAR(calc("sin(30)^2+cos(30)^2"), 1);  // 恒等式 sin^2+cos^2=1
}

// ============================================================================
// 4.1 自定义底数对数 log_a (双参数 log(底, 真数))
// 界面上的 log_a 按钮分两步输入: 底数 -> 真数, 最终组成 "log(a,b)" 交给求值核心。
// 以下测试直接验证求值核心对 "log(a,b)" 的语义。
// ============================================================================

TEST(log_custom_base_basic) {
    // log_a(b) = ln(b) / ln(a)
    CHECK_NEAR(calc("log(2,8)"), 3);
    CHECK_NEAR(calc("log(2,1024)"), 10);
    CHECK_NEAR(calc("log(3,27)"), 3);
    CHECK_NEAR(calc("log(5,25)"), 2);
    CHECK_NEAR(calc("log(10,100)"), 2);
    CHECK_NEAR(calc("log(4,16)"), 2);
}

TEST(log_custom_base_frac_decimal) {
    // 底数/真数为小数
    CHECK_NEAR(calc("log(2,16)"), 4);
    CHECK_NEAR(calc("log(0.5,0.25)"), 2);   // log_(0.5)(0.25) = 2
    CHECK_NEAR(calc("log(8,2)"), 1.0 / 3.0); // 底数 > 真数, 结果为分数
    CHECK_NEAR(calc("log(10,0.1)"), -1);    // 真数为小数
    CHECK_NEAR(calc("log(2,0.5)"), -1);     // 真数 < 1 得负数
}

TEST(log_custom_base_nested_expr) {
    // 底数/真数本身可以是复合表达式
    CHECK_NEAR(calc("log(1+1,2*4)"), 3);    // log_2(8)
    CHECK_NEAR(calc("log(3,3^3)"), 3);
    CHECK_NEAR(calc("log(sqrt(4),16)"), 4); // log_2(16)
    CHECK_NEAR(calc("log(2,8)*2"), 6);      // 与四则运算组合
}

TEST(log_custom_base_invalid) {
    CHECK_THROWS(calc("log(1,8)"));    // 底数 = 1
    CHECK_THROWS(calc("log(0,8)"));    // 底数 = 0
    CHECK_THROWS(calc("log(-2,8)"));   // 底数 < 0
    CHECK_THROWS(calc("log(2,0)"));    // 真数 = 0
    CHECK_THROWS(calc("log(2,-8)"));   // 真数 < 0
    CHECK_THROWS(calc("log(2,)"));     // 缺真数
    CHECK_THROWS(calc("log(,8)"));     // 缺底数
}

// ============================================================================
// 5. 空白字符 / 小数输入
// ============================================================================

TEST(whitespace) {
    CHECK_NEAR(calc("  3  +  4  "), 7);
    CHECK_NEAR(calc("  3\n+\t4  "), 7);
    CHECK_NEAR(calc("( 2 + 3 ) * 4"), 20);
    CHECK_NEAR(calc("sin ( 30 )"), 0.5);    // 函数名与括号间可留空格
    CHECK_NEAR(calc("log ( 2 , 8 )"), 3);
}

TEST(decimal_input) {
    CHECK_NEAR(calc(".5"), 0.5);            // 省略前导 0
    CHECK_NEAR(calc("1.5"), 1.5);
    CHECK_NEAR(calc("0.25+0.75"), 1);
    CHECK_NEAR(calc("3.14*2"), 6.28);
}

// ============================================================================
// 6. 数值精度(经典浮点场景)
// ============================================================================

TEST(fraction_accuracy) {
    CHECK_NEAR(calc("0.1+0.2"), 0.3);       // 经典浮点陷阱
    CHECK_NEAR(calc("0.3-0.1"), 0.2);
    CHECK_NEAR(calc("1/3*3"), 1);
    CHECK_NEAR(calc("10/3"), 10.0 / 3.0);
    CHECK_NEAR(calc("0.2*5"), 1);
}

TEST(big_numbers) {
    CHECK_NEAR(calc("123456789*987654321"), 123456789.0 * 987654321.0);
    CHECK_NEAR(calc("9999999999+1"), 10000000000.0);
    CHECK_NEAR(calc("2^40"), 1099511627776.0);
}

// ============================================================================
// 7. 异常处理
// ============================================================================

TEST(division_by_zero_throws) {
    CHECK_THROWS(calc("1/0"));
    CHECK_THROWS(calc("1/0.0"));
    CHECK_THROWS(calc("0/0"));
    CHECK_THROWS(calc("1/(2-2)"));
}

TEST(domain_errors_throw) {
    // 对数定义域
    CHECK_THROWS(calc("ln(0)"));
    CHECK_THROWS(calc("ln(-1)"));
    CHECK_THROWS(calc("log(0)"));
    CHECK_THROWS(calc("log(-5)"));
    // 开方定义域
    CHECK_THROWS(calc("sqrt(-4)"));
    CHECK_THROWS(calc("sqrt(-0.1)"));
    // 对数底数/真数非法
    CHECK_THROWS(calc("log(1,10)"));   // 底数 = 1
    CHECK_THROWS(calc("log(-1,10)"));  // 底数 < 0
    CHECK_THROWS(calc("log(0,10)"));   // 底数 = 0
    CHECK_THROWS(calc("log(2,0)"));    // 真数 = 0
    CHECK_THROWS(calc("log(2,-8)"));   // 真数 < 0
    // 负数底数的非整数次幂
    CHECK_THROWS(calc("(-8)^0.5"));
    CHECK_THROWS(calc("(-2)^0.3"));
}

TEST(syntax_errors_throw) {
    CHECK_THROWS(calc(""));            // 空表达式
    CHECK_THROWS(calc("   "));         // 纯空白
    CHECK_THROWS(calc("abc"));         // 非法标识符
    CHECK_THROWS(calc("1+"));          // 尾部缺操作数
    CHECK_THROWS(calc("*5"));          // 头部缺操作数
    CHECK_THROWS(calc("(1+2"));        // 缺右括号
    CHECK_THROWS(calc("1+2)"));        // 多余右括号
    CHECK_THROWS(calc("()"));          // 空括号
    CHECK_THROWS(calc("sin"));         // 函数后缺括号
    CHECK_THROWS(calc("sin(30"));      // 函数缺右括号
    CHECK_THROWS(calc("sin()"));       // 函数缺参数
    CHECK_THROWS(calc("foo(1)"));      // 未知函数
    CHECK_THROWS(calc("sin(1,2)"));    // 非 log 函数不允许两个参数
    CHECK_THROWS(calc("log(2,)"));     // log 缺第二个参数
    CHECK_THROWS(calc("1e10"));        // 不支持科学计数法
    CHECK_THROWS(calc("2x"));          // 数字后紧跟字母
}

TEST(error_messages) {
    expectThrowContains("1/0", "除数不能为零");
    expectThrowContains("sqrt(-4)", "sqrt");
    expectThrowContains("ln(-1)", "ln");
    expectThrowContains("log(1,10)", "对数底数");
    expectThrowContains("log(2,0)", "真数");
    expectThrowContains("(1+2", "右括号");
    expectThrowContains("1+2)", "无法识别");
    expectThrowContains("foo(1)", "未知函数");
    expectThrowContains("sin", "后需要括号");
    expectThrowContains("sin(1,2)", "只需一个参数");
    expectThrowContains("1e10", "无法识别");
    expectThrowContains("(-8)^0.5", "整数次幂");
}

// ============================================================================
// main: 运行全部测试
// ============================================================================

int main() {
    const auto& tests = registry();
    std::cout << "简易计算器单元测试: 共 " << tests.size() << " 个测试用例\n\n";

    int passed = 0;
    for (const auto& t : tests) {
        g_testFails = 0;
        std::cout << "[ RUN ] " << t.name << "\n";
        t.fn();
        if (g_testFails == 0) {
            ++passed;
            std::cout << "[ PASS] " << t.name << "\n";
        } else {
            std::cout << "[ FAIL] " << t.name << " (" << g_testFails << " 个检查失败)\n";
        }
        std::cout << "\n";
    }

    std::cout << "=============================================\n";
    std::cout << "用例: " << tests.size() << " 个, 通过 " << passed << " 个, 失败 "
              << (tests.size() - passed) << " 个\n";
    std::cout << "检查点: " << g_checks << " 个, 失败 " << g_failures << " 个\n";
    if (g_failures == 0) {
        std::cout << "结果: 全部通过\n";
        return 0;
    }
    std::cout << "结果: 存在失败\n";
    return 1;
}
