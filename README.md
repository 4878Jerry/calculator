# 🧮 简易计算器 Calculator

<p align="center">
  <img src="https://img.shields.io/badge/语言-C++17-blue" alt="C++17">
  <img src="https://img.shields.io/badge/平台-Windows%2010%2F11-lightgrey" alt="Windows">
  <img src="https://img.shields.io/badge/界面-原生%20Win32%20GUI-important" alt="Win32 GUI">
  <img src="https://img.shields.io/badge/依赖-零第三方库-success" alt="零依赖">
</p>

> 一个基于 **C++ / 原生 Win32** 的图形界面科学计算器。支持四则运算、括号、幂运算与常用数学函数,并内置历史记录管理;核心采用 **表达式二叉树(AST)+ 递归下降解析**,是「数据结构(树)+ 基础算法」的典型实践,界面零第三方依赖。

---

## Getting Started 快速开始

### Prerequisites 依赖项

| 项目 | 要求 |
|------|------|
| 操作系统 | Windows 10 / 11 |
| 编译器 | Visual Studio(v145 工具集)|
| C++ 标准 | C++17 |
| 第三方库 | 无(纯 Win32 API)|

### Installing 安装流程

**方式一:Visual Studio(推荐)**

1. 克隆仓库:`git clone https://github.com/4878Jerry/calculator.git`
2. 打开 `calculator/Calculator.sln`
3. 选择配置 `Debug` / `x64`
4. 按 `F5` 运行

**方式二:命令行 MSBuild**

```bash
git clone https://github.com/4878Jerry/calculator.git
cd calculator/calculator
MSBuild Calculator.sln -p:Configuration=Debug -p:Platform=x64
# 生成的可执行文件:x64\Debug\Calculator.exe
```

## Running the tests 运行测试

本项目未引入自动化测试框架,核心求值逻辑通过一组标准用例进行手工验证。运行 `Calculator.exe` 后输入表达式,应得到如下结果:

**正常用例**

| 表达式 | 期望结果 | 说明 |
|--------|:---:|------|
| `1+2*3` | 7 | 乘法优先 |
| `(1+2)*3` | 9 | 括号改变优先级 |
| `2^3^2` | 512 | 幂右结合 |
| `-2^2` | -4 | 一元负号优先级低于幂 |
| `2^-3` | 0.125 | 负指数 |
| `sin(30)` | 0.5 | 三角函数(角度制)|
| `log(100)` | 2 | 常用对数 |
| `log(2,8)` | 3 | 任意底数对数 |
| `2+3*sin(90)+4^2` | 21 | 复合表达式 |

**错误用例(应给出明确中文提示)**

| 表达式 | 期望提示 |
|--------|----------|
| `1/0` | 除数不能为零 |
| `sqrt(-1)` | sqrt 定义域要求 x ≥ 0 |
| `ln(0)` | ln 定义域要求 x > 0 |
| `log(1,5)` | 对数底数需 > 0 且 ≠ 1 |
| `(-2)^0.5` | 负数底数只能进行整数次幂 |
| `foo(3)` | 未知函数: foo |

## Usage 使用说明

### 基本操作
- **输入表达式**:点击数字、运算符、括号按钮,表达式实时显示在顶部显示框;
- **求值**:点击 `=`,结果显示在下方并自动存入历史;
- **清空 / 退格**:`C` 清空整条表达式,`←` 删除最后一个字符;
- **科学函数**:点击 `sin`、`cos`、`ln` 等按钮会自动插入 `sin(` 这样的带括号文本,补上参数和右括号后按 `=` 即可。

### 菜单
- **历史(H) → 查看历史…**:打开历史记录窗口,支持搜索、删除选中、清空;
- **历史(H) → 清空历史**:直接清空所有历史记录;
- **帮助(H) → 使用说明**:查看语法帮助。

### 支持的函数(API)

| 函数 | 说明 | 示例 |
|------|------|------|
| `sin(x)` `cos(x)` `tan(x)` | 三角函数(角度制)| `sin(30)` = 0.5 |
| `ln(x)` | 自然对数 | `ln(2.71828)` ≈ 1 |
| `log(x)` | 常用对数(log10)| `log(100)` = 2 |
| `log(底, 真数)` | 任意底数对数 | `log(2, 8)` = 3 |
| `exp(x)` | `e^x` | `exp(1)` = 2.71828… |
| `sqrt(x)` | 平方根 | `sqrt(16)` = 4 |

### 运算符优先级(从高到低)

| 优先级 | 运算符 / 语法 | 结合性 |
|:---:|---|---|
| 1(最高)| 数字、`( )`、函数调用 | — |
| 2 | `^` | 右结合 |
| 3 | 一元 `+` `-` | 右结合 |
| 4 | `*` `/` | 左结合 |
| 5(最低)| `+` `-` | 左结合 |

## Contributing 贡献者
4878Jerry
ssxk325
Chris3Brightman
hypppl
aaagjg

## Versioning 版本迭代

| 版本 | 说明 |
|------|------|
| v0.1 | 初始版本:基础四则运算、括号、一元正负号,由冀颢天负责 |
| v0.2 | 新增保存与查看历史记录,由符维磊负责|
| v0.3 | 扩展历史管理:单条删除、搜索、统计,由裴振羽负责 |
| v0.4 | 新增复杂计算:平方、指数、对数、三角函数(菜单版),由黄颍伦负责|
| v1.0.0 | 重构为 Win32 图形界面,扩展幂运算与科学函数,由胡腾飞负责 |
| v1.0.1 | 补充 README 介绍文档,由胡腾飞负责 |



## License 许可协议

本项目采用 [MIT License](https://opensource.org/licenses/MIT),可自由使用、修改与分发。

## Acknowledgments 致谢

- 感谢数据结构课程提供的实践机会,以及指导教师对项目的悉心指导;
- 界面与格式参考了标准 README 书写模板;
- 本项目为数据结构课程作业,欢迎通过 **Issue** 反馈问题、通过 **Pull Request** 提交改进;
- 提交前请确保代码风格与现有代码一致,并在 Windows 上验证编译通过。
