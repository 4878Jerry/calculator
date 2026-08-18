// 图形界面计算器 (Win32 GUI)
// 复用 calculator.h/.cpp 的表达式求值核心;历史记录沿用 history.txt 的「表达式 = 结果」格式。
#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>
#include "calculator.h"

namespace {

// ---------- 全局状态 ----------
HINSTANCE g_hInst = nullptr;
std::string g_expr;                  // 当前正在编辑的表达式(ASCII)

const char*    kHistoryFile = "history.txt";
const wchar_t* kMainClassName = L"CalcGuiMainWnd";
const wchar_t* kHistClassName = L"CalcGuiHistWnd";

// 窗口样式(不可最大化/拉伸)
const DWORD kMainStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
const DWORD kHistStyle  = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;

// 控件 ID
enum {
    IDC_DISPLAY = 100,
    IDC_RESULT  = 101,
    IDC_BTN_BASE = 200
};
enum {
    IDM_HISTORY_VIEW = 1000,
    IDM_HISTORY_CLEAR,
    IDM_HELP,
    IDM_EXIT
};
enum {
    IDH_SEARCH = 300,
    IDH_BTN_SEARCH,
    IDH_BTN_ALL,
    IDH_LIST,
    IDH_BTN_DELETE,
    IDH_BTN_CLEAR,
    IDH_BTN_CLOSE
};

// 布局尺寸
const int BTN_W = 84, BTN_H = 48, GAP = 4, MARGIN = 12;
const int COLS = 5, ROWS = 6;
const int DISPLAY_H = 36, RESULT_H = 22, SPACE = 8;
const int CLIENT_W = MARGIN * 2 + COLS * BTN_W + (COLS - 1) * GAP;
const int CLIENT_H = MARGIN + DISPLAY_H + SPACE + RESULT_H + SPACE + ROWS * BTN_H + (ROWS - 1) * GAP + MARGIN;

const int HIST_W = 400, HIST_H = 390;

// ---------- 编码转换(核心用 ASCII/UTF-8,界面用 UTF-16) ----------
std::wstring utf8ToWide(const std::string& s) {
    if (s.empty()) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::wstring w(static_cast<size_t>(n), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &w[0], n);
    w.resize(static_cast<size_t>(n - 1));
    return w;
}

std::string wideToUtf8(const std::wstring& w) {
    if (w.empty()) return "";
    int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string s(static_cast<size_t>(n), '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, &s[0], n, nullptr, nullptr);
    s.resize(static_cast<size_t>(n - 1));
    return s;
}

// ---------- 历史记录读写 ----------
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

void saveHistory(const std::vector<std::string>& lines) {
    std::ofstream out(kHistoryFile, std::ios::trunc);
    if (out.is_open()) {
        for (const auto& l : lines) out << l << '\n';
    }
}

void appendHistory(const std::string& expr, double result) {
    std::ofstream out(kHistoryFile, std::ios::app);
    if (out.is_open()) {
        out << expr << " = " << result << '\n';
    }
}

std::string formatNumber(double v) {
    if (std::isnan(v)) return "结果无效 (NaN)";
    if (std::isinf(v)) return v > 0 ? "正无穷 (+Inf)" : "负无穷 (-Inf)";
    std::ostringstream oss;
    oss.precision(12);
    oss << v;
    return oss.str();
}

// ---------- 按钮定义 ----------
enum BtnAction { ACT_INSERT, ACT_EQUAL, ACT_CLEAR, ACT_BACKSPACE };
struct ButtonDef {
    const wchar_t* label;   // 按钮上显示的文字
    const char*    text;    // ACT_INSERT 时追加到表达式的文本
    BtnAction      action;
};

const ButtonDef kButtons[COLS * ROWS] = {
    {L"sin", "sin(", ACT_INSERT},  {L"cos", "cos(", ACT_INSERT}, {L"tan", "tan(", ACT_INSERT}, {L"ln", "ln(", ACT_INSERT},  {L"log", "log(", ACT_INSERT},
    {L"exp", "exp(", ACT_INSERT},  {L"x²", "^2", ACT_INSERT},     {L"√", "sqrt(", ACT_INSERT}, {L"^", "^", ACT_INSERT},      {L"log_a", "log(", ACT_INSERT},
    {L"C", nullptr, ACT_CLEAR},    {L"←", nullptr, ACT_BACKSPACE},{L"(", "(", ACT_INSERT},       {L")", ")", ACT_INSERT},      {L"÷", "/", ACT_INSERT},
    {L"7", "7", ACT_INSERT},       {L"8", "8", ACT_INSERT},       {L"9", "9", ACT_INSERT},       {L"×", "*", ACT_INSERT},      {L"-", "-", ACT_INSERT},
    {L"4", "4", ACT_INSERT},       {L"5", "5", ACT_INSERT},       {L"6", "6", ACT_INSERT},       {L"+", "+", ACT_INSERT},      {L"=", nullptr, ACT_EQUAL},
    {L"1", "1", ACT_INSERT},       {L"2", "2", ACT_INSERT},       {L"3", "3", ACT_INSERT},       {L"0", "0", ACT_INSERT},      {L".", ".", ACT_INSERT},
};

// ---------- 主窗口 UI 辅助 ----------
void updateDisplay(HWND hwnd) {
    SetDlgItemTextW(hwnd, IDC_DISPLAY, utf8ToWide(g_expr).c_str());
}

void setResultText(HWND hwnd, const std::string& text) {
    SetDlgItemTextW(hwnd, IDC_RESULT, utf8ToWide(text).c_str());
}

void evaluateExpr(HWND hwnd) {
    if (g_expr.empty()) return;
    try {
        Calculator calc;
        double result = calc.evaluate(g_expr);
        setResultText(hwnd, "= " + formatNumber(result));
        appendHistory(g_expr, result);
    } catch (const std::exception& e) {
        setResultText(hwnd, std::string("错误: ") + e.what());
    }
}

void handleButton(HWND hwnd, int idx) {
    const ButtonDef& b = kButtons[idx];
    switch (b.action) {
        case ACT_INSERT:
            g_expr += b.text;
            setResultText(hwnd, "");
            updateDisplay(hwnd);
            break;
        case ACT_CLEAR:
            g_expr.clear();
            setResultText(hwnd, "");
            updateDisplay(hwnd);
            break;
        case ACT_BACKSPACE:
            if (!g_expr.empty()) g_expr.pop_back();
            updateDisplay(hwnd);
            break;
        case ACT_EQUAL:
            evaluateExpr(hwnd);
            break;
    }
}

// ---------- 历史窗口 ----------
void refreshHistoryList(HWND hwnd) {
    HWND list = GetDlgItem(hwnd, IDH_LIST);
    SendMessageW(list, LB_RESETCONTENT, 0, 0);
    auto recs = loadHistory();
    for (size_t i = 0; i < recs.size(); ++i) {
        std::wstring line = utf8ToWide(std::to_string(i + 1) + ". " + recs[i]);
        int idx = (int)SendMessageW(list, LB_ADDSTRING, 0, (LPARAM)line.c_str());
        SendMessageW(list, LB_SETITEMDATA, idx, (LPARAM)(i + 1));   // 记住原始序号
    }
}

void searchHistory(HWND hwnd) {
    wchar_t buf[256];
    GetDlgItemTextW(hwnd, IDH_SEARCH, buf, 256);
    std::string kw = wideToUtf8(buf);

    HWND list = GetDlgItem(hwnd, IDH_LIST);
    SendMessageW(list, LB_RESETCONTENT, 0, 0);
    auto recs = loadHistory();
    for (size_t i = 0; i < recs.size(); ++i) {
        if (recs[i].find(kw) != std::string::npos) {
            std::wstring line = utf8ToWide(std::to_string(i + 1) + ". " + recs[i]);
            int idx = (int)SendMessageW(list, LB_ADDSTRING, 0, (LPARAM)line.c_str());
            SendMessageW(list, LB_SETITEMDATA, idx, (LPARAM)(i + 1));
        }
    }
}

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

void clearHistory(HWND hwnd) {
    auto recs = loadHistory();
    if (recs.empty()) {
        MessageBoxW(hwnd, L"暂无历史记录。", L"提示", MB_ICONINFORMATION);
        return;
    }
    if (MessageBoxW(hwnd, L"确定要清空所有历史记录吗?", L"确认", MB_YESNO | MB_ICONWARNING) == IDYES) {
        saveHistory({});
        refreshHistoryList(hwnd);
    }
}

LRESULT CALLBACK HistWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CREATE: {
            CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                12, 12, 240, 26, hwnd, (HMENU)IDH_SEARCH, g_hInst, nullptr);
            CreateWindowW(L"BUTTON", L"搜索", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                256, 10, 64, 30, hwnd, (HMENU)IDH_BTN_SEARCH, g_hInst, nullptr);
            CreateWindowW(L"BUTTON", L"全部", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                324, 10, 64, 30, hwnd, (HMENU)IDH_BTN_ALL, g_hInst, nullptr);
            CreateWindowW(L"LISTBOX", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
                12, 48, 376, 290, hwnd, (HMENU)IDH_LIST, g_hInst, nullptr);
            CreateWindowW(L"BUTTON", L"删除选中", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                12, 346, 96, 32, hwnd, (HMENU)IDH_BTN_DELETE, g_hInst, nullptr);
            CreateWindowW(L"BUTTON", L"清空", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                116, 346, 96, 32, hwnd, (HMENU)IDH_BTN_CLEAR, g_hInst, nullptr);
            CreateWindowW(L"BUTTON", L"关闭", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                292, 346, 96, 32, hwnd, (HMENU)IDH_BTN_CLOSE, g_hInst, nullptr);
            refreshHistoryList(hwnd);
            return 0;
        }
        case WM_COMMAND: {
            int id = LOWORD(wp);
            switch (id) {
                case IDH_BTN_SEARCH: searchHistory(hwnd); break;
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
            // 历史窗口销毁时不退出应用
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

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

    // 手动模态循环
    MSG msg;
    while (IsWindow(hwndHist)) {
        if (!GetMessageW(&msg, nullptr, 0, 0)) break;
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    EnableWindow(owner, TRUE);
    SetForegroundWindow(owner);
}

// ---------- 主窗口 ----------
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CREATE: {
            // 表达式显示框
            CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_RIGHT | ES_READONLY,
                MARGIN, MARGIN, CLIENT_W - 2 * MARGIN, DISPLAY_H, hwnd, (HMENU)IDC_DISPLAY, g_hInst, nullptr);
            HFONT hFont = CreateFontW(22, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Consolas");
            SendMessageW(GetDlgItem(hwnd, IDC_DISPLAY), WM_SETFONT, (WPARAM)hFont, TRUE);

            // 结果行
            CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_RIGHT,
                MARGIN, MARGIN + DISPLAY_H + SPACE, CLIENT_W - 2 * MARGIN, RESULT_H, hwnd, (HMENU)IDC_RESULT, g_hInst, nullptr);

            // 按钮网格
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
            if (id >= IDC_BTN_BASE && id < IDC_BTN_BASE + COLS * ROWS) {
                handleButton(hwnd, id - IDC_BTN_BASE);
                return 0;
            }
            switch (id) {
                case IDM_HISTORY_VIEW:
                    showHistory(hwnd);
                    break;
                case IDM_HISTORY_CLEAR: {
                    auto recs = loadHistory();
                    if (recs.empty()) {
                        MessageBoxW(hwnd, L"暂无历史记录。", L"提示", MB_ICONINFORMATION);
                    } else if (MessageBoxW(hwnd, L"确定要清空所有历史记录吗?", L"确认", MB_YESNO | MB_ICONWARNING) == IDYES) {
                        saveHistory({});
                    }
                    break;
                }
                case IDM_HELP:
                    MessageBoxW(hwnd,
                        L"支持 + - * / ( ) ^ 与一元负号。\n"
                        L"函数: sin cos tan(角度)、ln、log(x) 或 log(底,真数)、exp、sqrt。\n\n"
                        L"示例: 3+4*2   sin(30)   2^10   log(2,8)",
                        L"使用说明", MB_OK | MB_ICONINFORMATION);
                    break;
                case IDM_EXIT:
                    DestroyWindow(hwnd);
                    break;
            }
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

} // namespace

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    g_hInst = hInstance;

    // 注册主窗口类
    WNDCLASSW wc = {};
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = kMainClassName;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClassW(&wc);

    // 注册历史窗口类
    WNDCLASSW wc2 = wc;
    wc2.lpfnWndProc = HistWndProc;
    wc2.lpszClassName = kHistClassName;
    RegisterClassW(&wc2);

    // 菜单
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

    // 主窗口
    RECT rc = { 0, 0, CLIENT_W, CLIENT_H };
    AdjustWindowRect(&rc, kMainStyle, TRUE);   // TRUE: 有菜单
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;

    HWND hwnd = CreateWindowW(kMainClassName, L"简易计算器 (GUI)",
        kMainStyle, CW_USEDEFAULT, CW_USEDEFAULT, w, h,
        nullptr, hMenu, hInstance, nullptr);
    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}
