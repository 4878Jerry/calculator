#include <iostream>
#include <cmath>
#include <limits>


using namespace std;

const double PI = 3.14159265358979323846;

// 将角度转换为弧度
double toRadians(double degrees) {
    return degrees * PI / 180.0;
}

// 显示菜单
void showMenu() {
    cout << "\n========== 多功能计算器 ==========\n";
    cout << " 1. 平方 (x^2)\n";
    cout << " 2. 正弦 (sin x)，输入角度\n";
    cout << " 3. 余弦 (cos x)，输入角度\n";
    cout << " 4. 正切 (tan x)，输入角度\n";
    cout << " 5. 指数 (e^x)\n";
    cout << " 6. 自然对数 (ln x)\n";
    cout << " 7. 常用对数 (log10 x)\n";
    cout << " 8. 自定义底数的指数 (a^x)\n";
    cout << " 9. 自定义底数的对数 (log_a x)\n";
    cout << "10. 退出\n";
    cout << "====================================\n";
    cout << "请选择操作 (1-10): ";
}

// 清空输入缓冲区
void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int main() {

    int choice;
    double x, a, result;

    cout << "欢迎使用多功能计算器！\n";

    while (true) {
        showMenu();
        cin >> choice;

        if (cin.fail()) {
            cout << "输入无效，请输入数字 1-10。\n";
            clearInput();
            continue;
        }

        if (choice == 10) {
            cout << "感谢使用，再见！\n";
            break;
        }

        if (choice < 1 || choice > 10) {
            cout << "无效选项，请重新选择。\n";
            continue;
        }

        // 根据选项执行计算
        switch (choice) {
        case 1: { // 平方
            cout << "请输入数值 x: ";
            cin >> x;
            if (cin.fail()) { cout << "输入无效。\n"; clearInput(); continue; }
            result = x * x;
            cout << x << " 的平方 = " << result << endl;
            break;
        }
        case 2: { // 正弦
            cout << "请输入角度 (度): ";
            cin >> x;
            if (cin.fail()) { cout << "输入无效。\n"; clearInput(); continue; }
            result = sin(toRadians(x));
            cout << "sin(" << x << "°) = " << result << endl;
            break;
        }
        case 3: { // 余弦
            cout << "请输入角度 (度): ";
            cin >> x;
            if (cin.fail()) { cout << "输入无效。\n"; clearInput(); continue; }
            result = cos(toRadians(x));
            cout << "cos(" << x << "°) = " << result << endl;
            break;
        }
        case 4: { // 正切
            cout << "请输入角度 (度): ";
            cin >> x;
            if (cin.fail()) { cout << "输入无效。\n"; clearInput(); continue; }
            result = tan(toRadians(x));
            cout << "tan(" << x << "°) = " << result << endl;
            break;
        }
        case 5: { // e^x
            cout << "请输入指数 x: ";
            cin >> x;
            if (cin.fail()) { cout << "输入无效。\n"; clearInput(); continue; }
            result = exp(x);
            cout << "e^" << x << " = " << result << endl;
            break;
        }
        case 6: { // 自然对数
            cout << "请输入真数 x (x > 0): ";
            cin >> x;
            if (cin.fail()) { cout << "输入无效。\n"; clearInput(); continue; }
            if (x <= 0) {
                cout << "错误：自然对数要求 x > 0。\n";
            }
            else {
                result = log(x);
                cout << "ln(" << x << ") = " << result << endl;
            }
            break;
        }
        case 7: { // 常用对数
            cout << "请输入真数 x (x > 0): ";
            cin >> x;
            if (cin.fail()) { cout << "输入无效。\n"; clearInput(); continue; }
            if (x <= 0) {
                cout << "错误：常用对数要求 x > 0。\n";
            }
            else {
                result = log10(x);
                cout << "log10(" << x << ") = " << result << endl;
            }
            break;
        }
        case 8: { // 自定义底数的指数 a^x
            cout << "请输入底数 a (建议 a > 0，但任意实数均可): ";
            cin >> a;
            if (cin.fail()) { cout << "输入无效。\n"; clearInput(); continue; }
            cout << "请输入指数 x: ";
            cin >> x;
            if (cin.fail()) { cout << "输入无效。\n"; clearInput(); continue; }
            // 如果底数为负且指数非整数，pow 可能返回 nan，这里不做额外处理，由 cmath 决定
            result = pow(a, x);
            cout << a << "^" << x << " = " << result << endl;
            break;
        }
        case 9: { // 自定义底数的对数 log_a(x)
            cout << "请输入底数 a (a > 0 且 a ≠ 1): ";
            cin >> a;
            if (cin.fail()) { cout << "输入无效。\n"; clearInput(); continue; }
            if (a <= 0 || a == 1) {
                cout << "错误：底数必须大于0且不等于1。\n";
                break;
            }
            cout << "请输入真数 x (x > 0): ";
            cin >> x;
            if (cin.fail()) { cout << "输入无效。\n"; clearInput(); continue; }
            if (x <= 0) {
                cout << "错误：真数必须大于0。\n";
                break;
            }
            // 换底公式：log_a(x) = ln(x) / ln(a)
            result = log(x) / log(a);
            cout << "log_" << a << "(" << x << ") = " << result << endl;
            break;
        }
        default:
            break;
        }
    }

    return 0;
}