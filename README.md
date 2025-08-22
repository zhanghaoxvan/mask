# Mask
<!-- 徽章 -->
[![License](https://img.shields.io/github/license/zhanghaoxvan/mask.svg)](https://github.com/zhanghaoxvan/mask)
[![Build Status](https://img.shields.io/cirrus/github/zhanghaoxvan/mask)](https://github.com/zhanghaoxvan/mask/actions)
[![GitHub Release](https://img.shields.io/github/v/release/zhanghaoxvan/mask)](https://github.com/zhanghaoxvan/mask/releases)

## 语言切换 / Change Languages

| 简体中文 | English |
| :------: | :-----: |
| [查看中文](#chinese-content) | [View English](#english-content) |

---

### **<a id="chinese-content">简体中文</a>**  

#### :rocket: 目标

1. 实现Mask语言的构建
2. 提供完善的包管理器

#### :building_construction: 架构

Mask采用
```mermaid
graph LR
Source-Code[源代码] --Lexer--> Token[Token流] --Parser--> AST[AST树] --CodeGen--> IR[LLVM IR] --LLVM--> Binary[二进制代码]
```
目录树图览：
```
mask/
├── CMakeLists.txt
├── LICENSE
├── README.md
├── cmake/
│   └── message.cmake // CMake编译时显示信息
├── example/
│   └── main.ma // 语法示例
├── inc/
│   ├── ast.hpp // AST树
│   ├── codegen.hpp // LLVM IR生成（todo）
│   ├── inc.hpp // 必须库文件
│   ├── lexer.hpp // 词法分析器
│   ├── parser.hpp // 语法分析器
│   └── preprocessor.hpp // 预处理器，记录import
├── lib/ // 库
│   ├── compile.ps1 // 编译
│   └── std
│       ├── fmt
│       │   ├── fmt.ma
│       │   ├── libfmt.cpp
│       │   ├── libfmt.hpp
│       │   └── libfmt.ll
│       └── str
│           ├── str.ma
│           ├── libstr.hpp
│           ├── libstr.cpp
│           └── libstr.ll
└── src/
    ├── ast.cpp
    ├── codegen.cpp
    ├── lexer.cpp
    ├── main.cpp
    ├── parser.cpp
    └── preprocessor.cpp
```

---

#### **<a id="english-content">English</a>**  

#### :rocket: target

1. Implement the construction of the Mask language
2. Provide a complete package manager

#### :building_construction: Architecture

Mask adopted
```mermaid
graph LR
Source-Code[Source Code] --Lexer--> Token[Token Stream] --Parser--> AST[AST] --CodeGen--> IR[LLVM IR] --LLVM--> Binary[Binary Code]
```
Directory tree overview:
```
mask/
├── CMakeLists.txt
├── LICENSE
├── README.md
├── cmake/
│   └── message.cmake // CMake displays information when compiling
├── example/
│   └── main.ma // Syntax examples
├── inc/
│   ├── ast.hpp // AST tree
│   ├── codegen.hpp // LLVM IR generation (todo)
│   ├── inc.hpp // Required library file
│   ├── lexer.hpp // Lexical Analyzer
│   ├── parser.hpp // Grammar Analyzer
│   └── preprocessor.hpp // Preprocessor, record import
├── lib/ // library
│   ├── compile.ps1 // Compilation
│   └── std
│   ├── fmt
│   │   ├── fmt.ma
│   │   ├── libfmt.cpp
│   │   ├── libfmt.hpp
│   │   └── libfmt.ll
│   └── str
│       ├── str.ma
│       ├── libstr.hpp
│       ├── libstr.cpp
│       └── libstr.ll
└── src/
    ├── ast.cpp
    ├── codegen.cpp
    ├── lexer.cpp
    ├── main.cpp
    ├── parser.cpp
    └── preprocessor.cpp
```
