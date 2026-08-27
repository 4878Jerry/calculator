/**
 * @file    main.cpp
 * @brief   图形界面计算器 (Win32 GUI)
 * @author  裴振羽
 * @date    2026-08-27
 * @version 1.0.0
 * 
 * @details 基于 Win32 API 的图形界面计算器。
 *          核心计算复用 calculator.h/calculator.cpp 的表达式求值模块。
 * 
 *          主要功能：
 *          - 可视化按钮输入（数字、运算符、函数）
 *          - 实时显示表达式和计算结果
 *          - 历史记录管理（查看、搜索、删除、清空）
 *          - 菜单栏（历史管理、使用说明）
 * 
 *          历史记录存储：history.txt（与表达式同一目录）
 *          格式：每行为 "表达式 = 结果"
 * 
 * @note    采用 Unicode (UTF-16) 作为窗口字符串编码，
 *          内部核心使用 UTF-8/ASCII，通过 utf8ToWide/wideToUtf8 转换。
 */

#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>
#include "calculator.h"

namespace {

<<<<<<< HEAD
// 自定义底数对数(log_a)的分步输入状态。
// 用户依次输入: log_a -> 底数 -> log_a -> 真数 -> ')' -> '='。
// 第一次按下 log_a 输入 "log(", 等待用户输入底数;
// 第二次按下 log_a 输入 ",", 分隔底数与真数。
enum class LogInputStage {
    Normal,        // 不在 log_a 输入流程中
    AwaitingBase,  // 已输入 "log(", 等待底数及分隔逗号
};

// ---------- 全局状态 ----------
HINSTANCE g_hInst = nullptr;
std::string g_expr;                  // 当前正在编辑的表达式(ASCII)
HFONT g_hDisplayFont = nullptr;      // 表达式显示框字体(WM_DESTROY 时释放)
LogInputStage g_logStage = LogInputStage::Normal;  // log_a 分步输入状态
=======
// ===================== 全局状态 =====================
>>>>>>> e165bd5 (docs: 为第三次作业添加完整的 Doxygen 风格代码注释)

HINSTANCE g_hInst = nullptr;       ///< 应用程序实例句柄
std::string g_expr;                ///< 当前正在编辑的表达式 (UTF-8/ASCII)

// ===================== 常量定义 =====================

const char*    kHistoryFile = "history.txt";           ///< 历史记录文件名
const wchar_t* kMainClassName = L"CalcGuiMainWnd";     ///< 主窗口类名
const wchar_t* kHistClassName = L"CalcGuiHistWnd";     ///< 历史记录窗口类名

/// 主窗口样式：不可最大化、不可拉伸
const DWORD kMainStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
const DWORD kHistStyle  = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;  ///< 历史窗口样式

// ===================== 控件 ID 定义 =====================

enum {
    IDC_DISPLAY = 100,    ///< 表达式显示框
    IDC_RESULT  = 101,    ///< 结果显示框
    IDC_BTN_BASE = 200    ///< 按钮 ID 基值
};

enum {
    IDM_HISTORY_VIEW = 1000,  ///< 菜单：查看历史
    IDM_HISTORY_CLEAR,        ///< 菜单：清空历史
    IDM_HELP,                 ///< 菜单：使用说明
    IDM_EXIT                  ///< 菜单：退出
};

enum {
    IDH_SEARCH = 300,         ///< 搜索框
    IDH_BTN_SEARCH,           ///< 搜索按钮
    IDH_BTN_ALL,              ///< 全部按钮
    IDH_LIST,                 ///< 历史列表
    IDH_BTN_DELETE,           ///< 删除按钮
    IDH_BTN_CLEAR,            ///< 清空按钮
    IDH_BTN_CLOSE             ///< 关闭按钮
};

// ===================== 布局尺寸 =====================

const int BTN_W = 84, BTN_H = 48;          ///< 按钮宽度/高度
const int GAP = 4, MARGIN = 12;            ///< 间距/边距
const int COLS = 5, ROWS = 6;              ///< 按钮网格列数/行数
const int DISPLAY_H = 36, RESULT_H = 22;   ///< 显示框/结果框高度
const int SPACE = 8;                       ///< 垂直间隔

/// 主窗口客户区宽度 = 边距*2 + 列数*按钮宽 + (列数-1)*间距
const int CLIENT_W = MARGIN * 2 + COLS * BTN_W + (COLS - 1) * GAP;
/// 主窗口客户区高度 = 上边距 + 显示框 + 间隔 + 结果框 + 间隔 + 按钮网格 + 下边距
const int CLIENT_H = MARGIN + DISPLAY_H + SPACE + RESULT_H + SPACE + ROWS * BTN_H + (ROWS - 1) * GAP + MARGIN;

<<<<<<< HEAD
// 历史窗口布局
const int HIST_W = 400, HIST_H = 390;
const int HIST_MARGIN = 12;
const int HIST_SEARCH_W = 240, HIST_SEARCH_H = 26;
const int HIST_SEARCH_BTN_X = HIST_MARGIN + HIST_SEARCH_W + 4;   // 256
const int HIST_BTN_W = 64, HIST_BTN_H = 30;
const int HIST_LIST_TOP = HIST_MARGIN + HIST_SEARCH_H + 10;      // 48
const int HIST_LIST_W = HIST_W - 2 * HIST_MARGIN;                // 376
const int HIST_LIST_H = 290;
const int HIST_ACTION_Y = HIST_LIST_TOP + HIST_LIST_H + 8;       // 346
const int HIST_ACTION_W = 96, HIST_ACTION_H = 32;
=======
const int HIST_W = 400, HIST_H = 390;      ///< 历史窗口宽/高
>>>>>>> e165bd5 (docs: 为第三次作业添加完整的 Doxygen 风格代码注释)

// ===================== 编码转换工具 =====================

/**
 * @brief   UTF-8 字符串转 UTF-16 宽字符串
 * @param   s   UTF-8 编码的字符串
 * @return  UTF-16 编码的宽字符串
 */
std::wstring utf8ToWide(const std::string& s) {
    if (s.empty()) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::wstring w(static_cast<size_t>(n), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &w[0], n);
    w.resize(static_cast<size_t>(n - 1));
    return w;
}

/**
 * @brief   UTF-16 宽字符串转 UTF-8 字符串
 * @param   w   UTF-16 编码的宽字符串
 * @return  UTF-8 编码的字符串
 */
std::string wideToUtf8(const std::wstring& w) {
    if (w.empty()) return "";
    int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string s(static_cast<size_t>(n), '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, &s[0], n, nullptr, nullptr);
    s.resize(static_cast<size_t>(n - 1));
    return s;
}

// ===================== 历史记录管理 =====================

/**
 * @brief   从文件加载历史记录
 * @return  包含所有历史记录行的 vector，按存储顺序排列
 * @note    如果文件不存在，返回空 vector
 */
std::vector<std::string> loadHistory() {
    std::vector<std::string> lines;
    std::ifstream in(kHistoryFile);
    if (!in.is_open()) return lines;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty()) lines.push_back(line);
    }
    return lines;
}

/**
 * @brief   保存历史记录到文件（覆盖写入）
 * @param   lines   要保存的历史记录行
 */
void saveHistory(const std::vector<std::string>& lines) {
    std::ofstream out(kHistoryFile, std::ios::trunc);
    if (out.is_open()) {
        for (const auto& l : lines) out << l << '\n';
    }
}

/**
 * @brief   追加一条历史记录
 * @param   expr    表达式
 * @param   result  计算结果
 */
void appendHistory(const std::string& expr, double result) {
    std::ofstream out(kHistoryFile, std::ios::app);
    if (out.is_open()) {
        out << expr << " = " << result << '\n';
    }
}

/**
 * @brief   格式化数字显示
 * @param   v   要格式化的数值
 * @return  格式化的字符串
 * @note    - NaN 显示为 "结果无效 (NaN)"
 *          - ±Inf 显示为 "正无穷 (+Inf)" / "负无穷 (-Inf)"
 *          - 普通浮点数保留 12 位有效数字
 */
std::string formatNumber(double v) {
    if (std::isnan(v)) return "结果无效 (NaN)";
    if (std::isinf(v)) return v > 0 ? "正无穷 (+Inf)" : "负无穷 (-Inf)";
    std::ostringstream oss;
    oss.precision(12);
    oss << v;
    return oss.str();
}

<<<<<<< HEAD
// ---------- 按钮定义 ----------
// 按钮动作类型: 决定按下该按钮时 UI 层如何处理。
enum class BtnAction {
    Insert,      // 直接向表达式追加一段文本
    Equal,       // 计算当前表达式
    Clear,       // 清空表达式
    Backspace,   // 删除最后一个字符
    LogBase,     // 自定义底数对数(log_a): 按状态插入 "log(" 或分隔逗号 ","
};

struct ButtonDef {
    const wchar_t* label;   // 按钮上显示的文字
    const char*    text;    // BtnAction::Insert 时追加到表达式的文本
    BtnAction      action;
};

// 6 行 × 5 列按钮布局。其中 "lg" 为常用对数(以 10 为底),
// "log_a" 为自定义底数对数, 需分两步输入底数与真数。
=======
// ===================== 按钮定义 =====================

/**
 * @enum    BtnAction
 * @brief   按钮动作类型
 */
enum BtnAction { 
    ACT_INSERT,     ///< 插入文本到表达式
    ACT_EQUAL,      ///< 执行求值
    ACT_CLEAR,      ///< 清空表达式
    ACT_BACKSPACE   ///< 退格删除
};

/**
 * @struct  ButtonDef
 * @brief   按钮配置结构
 */
struct ButtonDef {
    const wchar_t* label;   ///< 按钮显示的文本
    const char*    text;    ///< ACT_INSERT 时追加到表达式的文本
    BtnAction      action;  ///< 动作类型
};

/**
 * @var kButtons
 * @brief   按钮网格定义（按行顺序排列，共 5 列 × 6 行）
 * @details 布局：
 *          第1行：sin  cos  tan  ln   log
 *          第2行：exp  x²   √    ^    log_a
 *          第3行：C    ←    (    )    ÷
 *          第4行：7    8    9    ×    -
 *          第5行：4    5    6    +    =
 *          第6行：1    2    3    0    .
 */
>>>>>>> e165bd5 (docs: 为第三次作业添加完整的 Doxygen 风格代码注释)
const ButtonDef kButtons[COLS * ROWS] = {
    {L"sin",   "sin(",   BtnAction::Insert},  {L"cos", "cos(", BtnAction::Insert}, {L"tan", "tan(", BtnAction::Insert}, {L"ln",   "ln(",   BtnAction::Insert}, {L"lg",    "log(",  BtnAction::Insert},
    {L"exp",   "exp(",   BtnAction::Insert},  {L"x²",  "^2",   BtnAction::Insert}, {L"√",    "sqrt(", BtnAction::Insert}, {L"^",    "^",     BtnAction::Insert}, {L"log_a", nullptr, BtnAction::LogBase},
    {L"C",     nullptr,  BtnAction::Clear},   {L"←",   nullptr, BtnAction::Backspace},{L"(",   "(",     BtnAction::Insert}, {L")",    ")",     BtnAction::Insert}, {L"÷",    "/",     BtnAction::Insert},
    {L"7",     "7",      BtnAction::Insert},  {L"8",   "8",     BtnAction::Insert}, {L"9",    "9",     BtnAction::Insert}, {L"×",    "*",     BtnAction::Insert}, {L"-",    "-",     BtnAction::Insert},
    {L"4",     "4",      BtnAction::Insert},  {L"5",   "5",     BtnAction::Insert}, {L"6",    "6",     BtnAction::Insert}, {L"+",    "+",     BtnAction::Insert}, {L"=",    nullptr, BtnAction::Equal},
    {L"1",     "1",      BtnAction::Insert},  {L"2",   "2",     BtnAction::Insert}, {L"3",    "3",     BtnAction::Insert}, {L"0",    "0",     BtnAction::Insert}, {L".",    ".",     BtnAction::Insert},
};

// ===================== 主窗口 UI 辅助函数 =====================

/**
 * @brief   更新表达式显示
 * @param   hwnd    主窗口句柄
 */
void updateDisplay(HWND hwnd) {
    SetDlgItemTextW(hwnd, IDC_DISPLAY, utf8ToWide(g_expr).c_str());
}

/**
 * @brief   设置结果文本
 * @param   hwnd    主窗口句柄
 * @param   text    要显示的文本
 */
void setResultText(HWND hwnd, const std::string& text) {
    SetDlgItemTextW(hwnd, IDC_RESULT, utf8ToWide(text).c_str());
}

<<<<<<< HEAD
// 向表达式追加文本并刷新显示。
// 追加内容后旧的结算结果不再有效, 因此一并清空结果框。
void insertText(HWND hwnd, const std::string& text) {
    if (text.empty()) return;
    g_expr += text;
    setResultText(hwnd, "");
    updateDisplay(hwnd);
}

// 清空当前表达式并刷新显示。
void clearExpression(HWND hwnd) {
    g_expr.clear();
    setResultText(hwnd, "");
    updateDisplay(hwnd);
}

=======
/**
 * @brief   执行表达式求值
 * @param   hwnd    主窗口句柄
 * @note    求值成功后自动追加到历史记录
 */
>>>>>>> e165bd5 (docs: 为第三次作业添加完整的 Doxygen 风格代码注释)
void evaluateExpr(HWND hwnd) {
    if (g_expr.empty()) return;
    static Calculator calc;   // 核心求值器无状态, 复用单个实例
    try {
        double result = calc.evaluate(g_expr);
        setResultText(hwnd, "= " + formatNumber(result));
        appendHistory(g_expr, result);
    } catch (const std::exception& e) {
        setResultText(hwnd, std::string("错误: ") + e.what());
    }
}

<<<<<<< HEAD
// 是否属于 log_a 分步输入中的"数字/小数点"按键。
// 只有在输入底数或真数的数字时, log_a 的输入状态才保持不变,
// 这样第二次按下 log_a 才会在底数后补上逗号。
bool isDigitOrDot(const char* text) {
    if (!text) return false;
    return std::strchr("0123456789.", text[0]) != nullptr;
}

// 按下 "log_a" 按钮: 完成自定义底数对数的分步输入。
//   第一次按下 -> 追加 "log(", 进入"等待底数"状态;
//   第二次按下 -> 追加 "," 分隔底数与真数, 退出该状态。
void handleLogBase(HWND hwnd) {
    if (g_logStage == LogInputStage::AwaitingBase) {
        insertText(hwnd, ",");
        g_logStage = LogInputStage::Normal;
    } else {
        insertText(hwnd, "log(");
        g_logStage = LogInputStage::AwaitingBase;
    }
}

// 计算结束(log_a 的 ')' 与 '=' 已按下)后重置分步输入状态。
void resetLogStageIfNeeded() {
    if (g_logStage != LogInputStage::Normal)
        g_logStage = LogInputStage::Normal;
}

// 统一处理一次按钮点击。
=======
/**
 * @brief   处理按钮点击事件
 * @param   hwnd    主窗口句柄
 * @param   idx     按钮索引 (0 ~ COLS*ROWS-1)
 */
>>>>>>> e165bd5 (docs: 为第三次作业添加完整的 Doxygen 风格代码注释)
void handleButton(HWND hwnd, int idx) {
    const ButtonDef& b = kButtons[idx];
    switch (b.action) {
        case BtnAction::Insert: {
            // 输入数字/小数点时不打断 log_a 分步状态; 其余输入会退出该状态
            if (!isDigitOrDot(b.text))
                resetLogStageIfNeeded();
            insertText(hwnd, b.text ? b.text : "");
            break;
        }
        case BtnAction::LogBase:
            handleLogBase(hwnd);
            break;
        case BtnAction::Clear:
            resetLogStageIfNeeded();
            clearExpression(hwnd);
            break;
        case BtnAction::Backspace:
            if (!g_expr.empty()) g_expr.pop_back();
            resetLogStageIfNeeded();
            updateDisplay(hwnd);
            break;
        case BtnAction::Equal:
            resetLogStageIfNeeded();
            evaluateExpr(hwnd);
            break;
    }
}

<<<<<<< HEAD
// ---------- 历史窗口 ----------
// 把历史记录填充到列表;keyword 非空时只保留包含关键字的记录。
// 行首序号(i+1)是记录在文件中的原始位置, 删除时据此定位。
void populateHistoryList(HWND hwnd, const std::string& keyword) {
=======
// ===================== 历史记录窗口 =====================

/**
 * @brief   刷新历史记录列表
 * @param   hwnd    历史窗口句柄
 */
void refreshHistoryList(HWND hwnd) {
>>>>>>> e165bd5 (docs: 为第三次作业添加完整的 Doxygen 风格代码注释)
    HWND list = GetDlgItem(hwnd, IDH_LIST);
    SendMessageW(list, LB_RESETCONTENT, 0, 0);
    auto recs = loadHistory();
    for (size_t i = 0; i < recs.size(); ++i) {
        if (!keyword.empty() && recs[i].find(keyword) == std::string::npos)
            continue;
        std::wstring line = utf8ToWide(std::to_string(i + 1) + ". " + recs[i]);
        int idx = (int)SendMessageW(list, LB_ADDSTRING, 0, (LPARAM)line.c_str());
        SendMessageW(list, LB_SETITEMDATA, idx, (LPARAM)(i + 1));
    }
}

<<<<<<< HEAD
void refreshHistoryList(HWND hwnd) { populateHistoryList(hwnd, ""); }
=======
/**
 * @brief   搜索历史记录
 * @param   hwnd    历史窗口句柄
 * @note    根据搜索框内容筛选列表，显示包含关键词的记录
 */
void searchHistory(HWND hwnd) {
    wchar_t buf[256];
    GetDlgItemTextW(hwnd, IDH_SEARCH, buf, 256);
    std::string kw = wideToUtf8(buf);
>>>>>>> e165bd5 (docs: 为第三次作业添加完整的 Doxygen 风格代码注释)

// 弹出确认框询问是否清空历史;返回是否已确认清空。
bool confirmClearHistory(HWND hwnd) {
    if (loadHistory().empty()) {
        MessageBoxW(hwnd, L"暂无历史记录。", L"提示", MB_ICONINFORMATION);
        return false;
    }
    return MessageBoxW(hwnd, L"确定要清空所有历史记录吗?", L"确认", MB_YESNO | MB_ICONWARNING) == IDYES;
}

/**
 * @brief   删除选中的历史记录
 * @param   hwnd    历史窗口句柄
 */
void deleteSelectedHistory(HWND hwnd) {
    HWND list = GetDlgItem(hwnd, IDH_LIST);
    int sel = (int)SendMessageW(list, LB_GETCURSEL, 0, 0);
    if (sel == LB_ERR) {
        MessageBoxW(hwnd, L"请先在列表中选择一条记录。", L"提示", MB_ICONINFORMATION);
        return;
    }
    int orig = (int)SendMessageW(list, LB_GETITEMDATA, sel, 0);
    auto recs = loadHistory();
    if (orig >= 1 && orig <= (int)recs.size()) {
        recs.erase(recs.begin() + (orig - 1));
        saveHistory(recs);
    }
    refreshHistoryList(hwnd);
}

/**
 * @brief   清空所有历史记录
 * @param   hwnd    历史窗口句柄
 * @note    操作前会弹出确认对话框
 */
void clearHistory(HWND hwnd) {
    if (confirmClearHistory(hwnd)) {
        saveHistory({});
        refreshHistoryList(hwnd);
    }
}

/**
 * @brief   历史记录窗口过程
 * @param   hwnd    窗口句柄
 * @param   msg     消息
 * @param   wp      参数
 * @param   lp      参数
 * @return  消息处理结果
 */
LRESULT CALLBACK HistWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CREATE: {
            // 搜索框
            CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                HIST_MARGIN, HIST_MARGIN, HIST_SEARCH_W, HIST_SEARCH_H,
                hwnd, (HMENU)IDH_SEARCH, g_hInst, nullptr);
            CreateWindowW(L"BUTTON", L"搜索", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                HIST_SEARCH_BTN_X, HIST_MARGIN - 2, HIST_BTN_W, HIST_BTN_H,
                hwnd, (HMENU)IDH_BTN_SEARCH, g_hInst, nullptr);
            CreateWindowW(L"BUTTON", L"全部", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
<<<<<<< HEAD
                HIST_SEARCH_BTN_X + HIST_BTN_W + 4, HIST_MARGIN - 2, HIST_BTN_W, HIST_BTN_H,
                hwnd, (HMENU)IDH_BTN_ALL, g_hInst, nullptr);
            CreateWindowW(L"LISTBOX", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
                HIST_MARGIN, HIST_LIST_TOP, HIST_LIST_W, HIST_LIST_H,
                hwnd, (HMENU)IDH_LIST, g_hInst, nullptr);
=======
                324, 10, 64, 30, hwnd, (HMENU)IDH_BTN_ALL, g_hInst, nullptr);
            // 历史列表
            CreateWindowW(L"LISTBOX", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
                12, 48, 376, 290, hwnd, (HMENU)IDH_LIST, g_hInst, nullptr);
            // 操作按钮
>>>>>>> e165bd5 (docs: 为第三次作业添加完整的 Doxygen 风格代码注释)
            CreateWindowW(L"BUTTON", L"删除选中", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                HIST_MARGIN, HIST_ACTION_Y, HIST_ACTION_W, HIST_ACTION_H,
                hwnd, (HMENU)IDH_BTN_DELETE, g_hInst, nullptr);
            CreateWindowW(L"BUTTON", L"清空", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                HIST_MARGIN + HIST_ACTION_W + 8, HIST_ACTION_Y, HIST_ACTION_W, HIST_ACTION_H,
                hwnd, (HMENU)IDH_BTN_CLEAR, g_hInst, nullptr);
            CreateWindowW(L"BUTTON", L"关闭", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                HIST_W - HIST_MARGIN - HIST_ACTION_W, HIST_ACTION_Y, HIST_ACTION_W, HIST_ACTION_H,
                hwnd, (HMENU)IDH_BTN_CLOSE, g_hInst, nullptr);
            refreshHistoryList(hwnd);
            return 0;
        }
        case WM_COMMAND: {
            int id = LOWORD(wp);
            switch (id) {
                case IDH_BTN_SEARCH: {
                    wchar_t buf[256];
                    GetDlgItemTextW(hwnd, IDH_SEARCH, buf, 256);
                    populateHistoryList(hwnd, wideToUtf8(buf));
                    break;
                }
                case IDH_BTN_ALL:    refreshHistoryList(hwnd); break;
                case IDH_BTN_DELETE: deleteSelectedHistory(hwnd); break;
                case IDH_BTN_CLEAR:  clearHistory(hwnd); break;
                case IDH_BTN_CLOSE:  DestroyWindow(hwnd); break;
            }
            return 0;
        }
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            // 不退出应用，仅销毁窗口
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

/**
 * @brief   显示历史记录窗口（模态对话框）
 * @param   owner   父窗口句柄
 */
void showHistory(HWND owner) {
    RECT rcOwner;
    GetWindowRect(owner, &rcOwner);

    RECT rc = { 0, 0, HIST_W, HIST_H };
    AdjustWindowRect(&rc, kHistStyle, FALSE);
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;
    int x = rcOwner.left + ((rcOwner.right - rcOwner.left) - w) / 2;
    int y = rcOwner.top + ((rcOwner.bottom - rcOwner.top) - h) / 2;

    HWND hwndHist = CreateWindowW(kHistClassName, L"历史记录",
        kHistStyle, x, y, w, h, owner, nullptr, g_hInst, nullptr);
    if (!hwndHist) return;

    ShowWindow(hwndHist, SW_SHOW);
    EnableWindow(owner, FALSE);

    // 手动模态消息循环
    MSG msg;
    while (IsWindow(hwndHist)) {
        if (!GetMessageW(&msg, nullptr, 0, 0)) break;
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    EnableWindow(owner, TRUE);
    SetForegroundWindow(owner);
}

// ===================== 主窗口 =====================

/**
 * @brief   主窗口过程
 * @param   hwnd    窗口句柄
 * @param   msg     消息
 * @param   wp      参数
 * @param   lp      参数
 * @return  消息处理结果
 */
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CREATE: {
            // 表达式显示框（只读右对齐）
            CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_RIGHT | ES_READONLY,
                MARGIN, MARGIN, CLIENT_W - 2 * MARGIN, DISPLAY_H, hwnd, (HMENU)IDC_DISPLAY, g_hInst, nullptr);
            g_hDisplayFont = CreateFontW(22, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Consolas");
            SendMessageW(GetDlgItem(hwnd, IDC_DISPLAY), WM_SETFONT, (WPARAM)g_hDisplayFont, TRUE);

            // 结果框
            CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_RIGHT,
                MARGIN, MARGIN + DISPLAY_H + SPACE, CLIENT_W - 2 * MARGIN, RESULT_H, hwnd, (HMENU)IDC_RESULT, g_hInst, nullptr);

            // 创建按钮网格
            int by = MARGIN + DISPLAY_H + SPACE + RESULT_H + SPACE;
            for (int r = 0; r < ROWS; ++r) {
                for (int c = 0; c < COLS; ++c) {
                    int i = r * COLS + c;
                    int x = MARGIN + c * (BTN_W + GAP);
                    int y = by + r * (BTN_H + GAP);
                    CreateWindowW(L"BUTTON", kButtons[i].label, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                        x, y, BTN_W, BTN_H, hwnd, (HMENU)(INT_PTR)(IDC_BTN_BASE + i), g_hInst, nullptr);
                }
            }
            return 0;
        }
        case WM_COMMAND: {
            int id = LOWORD(wp);
            // 按钮点击事件
            if (id >= IDC_BTN_BASE && id < IDC_BTN_BASE + COLS * ROWS) {
                handleButton(hwnd, id - IDC_BTN_BASE);
                return 0;
            }
            // 菜单事件
            switch (id) {
                case IDM_HISTORY_VIEW:
                    showHistory(hwnd);
                    break;
                case IDM_HISTORY_CLEAR:
                    // 主窗口无历史列表, 确认后仅清空文件即可
                    if (confirmClearHistory(hwnd))
                        saveHistory({});
                    break;
                case IDM_HELP:
                    MessageBoxW(hwnd,
                        L"支持 + - * / ( ) ^ 与一元负号。\n"
                        L"函数: sin cos tan(角度)、ln、exp、sqrt。\n"
                        L"lg: 常用对数(以 10 为底)。\n"
                        L"log_a: 自定义底数对数, 分两步输入底数与真数:\n"
                        L"  按 log_a → 输入底数 → 再按 log_a → 输入真数 → ) → =\n\n"
                        L"示例: 3+4*2   sin(30)   2^10   lg(100)   log_a 2 → log_a 8 → ) → =(得3)",
                        L"使用说明", MB_OK | MB_ICONINFORMATION);
                    break;
                case IDM_EXIT:
                    DestroyWindow(hwnd);
                    break;
            }
            return 0;
        }
        case WM_DESTROY:
            if (g_hDisplayFont) {
                DeleteObject(g_hDisplayFont);
                g_hDisplayFont = nullptr;
            }
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

} // namespace

// ===================== 程序入口 =====================

/**
 * @brief   应用程序入口点
 * @param   hInstance       应用程序实例句柄
 * @param   hPrevInstance   前一实例句柄（Win32 中始终为 NULL）
 * @param   lpCmdLine       命令行参数（未使用）
 * @param   nCmdShow        窗口显示方式
 * @return  退出码
 * 
 * @details 执行流程：
 *          1. 注册主窗口类和历史窗口类
 *          2. 创建菜单栏
 *          3. 创建主窗口
 *          4. 进入消息循环
 * 
 * @note    Windows 环境下，GUI 应用程序的入口应为 wWinMain（Unicode 版本）
 */
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    g_hInst = hInstance;

    // 注册主窗口类
    WNDCLASSW wc = {};
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = kMainClassName;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    if (!RegisterClassW(&wc)) return 0;

    // 注册历史窗口类
    WNDCLASSW wc2 = wc;
    wc2.lpfnWndProc = HistWndProc;
    wc2.lpszClassName = kHistClassName;
    if (!RegisterClassW(&wc2)) return 0;

    // 创建菜单栏
    HMENU hMenu = CreateMenu();

    HMENU hHistMenu = CreatePopupMenu();
    AppendMenuW(hHistMenu, MF_STRING, IDM_HISTORY_VIEW, L"查看历史...");
    AppendMenuW(hHistMenu, MF_STRING, IDM_HISTORY_CLEAR, L"清空历史");
    AppendMenuW(hHistMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hHistMenu, MF_STRING, IDM_EXIT, L"退出");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hHistMenu, L"历史(H)");

    HMENU hHelpMenu = CreatePopupMenu();
    AppendMenuW(hHelpMenu, MF_STRING, IDM_HELP, L"使用说明");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hHelpMenu, L"帮助(H)");

    // 计算窗口尺寸
    RECT rc = { 0, 0, CLIENT_W, CLIENT_H };
    AdjustWindowRect(&rc, kMainStyle, TRUE);
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;

    // 创建主窗口
    HWND hwnd = CreateWindowW(kMainClassName, L"简易计算器 (GUI)",
        kMainStyle, CW_USEDEFAULT, CW_USEDEFAULT, w, h,
        nullptr, hMenu, hInstance, nullptr);
    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // 主消息循环
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}