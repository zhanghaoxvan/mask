# Mask现状

[https://github.com/zhanghaoxvan/mask](Mask)现已迁移至[https://github.com/zhanghaoxvan/Gobol](Gobol).

## Warning Gobol的语法与Mask有差异

# Mask
<!-- 徽章 -->
[![License](https://img.shields.io/github/license/zhanghaoxvan/mask.svg)](https://github.com/zhanghaoxvan/mask)
[![Build Status](https://img.shields.io/cirrus/github/zhanghaoxvan/mask)](https://github.com/zhanghaoxvan/mask/actions)
[![GitHub Release](https://img.shields.io/github/v/release/zhanghaoxvan/mask)](https://github.com/zhanghaoxvan/mask/releases)

## :rocket: 目标

1. 实现Mask语言的构建
2. 提供完善的包管理器

## :building_construction: 架构

Mask采用
```mermaid
graph LR
Source-Code[源代码] --Lexer--> Token[Token流] --Parser--> AST[AST树] --VMGen--> VM-File[VM文件]

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
│   ├── vmgen.hpp // Mask VM生存 (todo)
│   ├── inc.hpp // 必须库文件
│   ├── lexer.hpp // 词法分析器
│   ├── parser.hpp // 语法分析器
│   └── mask-vm-parser.hpp // Mask VM解析器
├── lib/ // 库默认引用的库
└── src/
    ├── ast.cpp
    ├── codegen.cpp
    ├── lexer.cpp
    ├── main.cpp
    ├── parser.cpp
    └── mask-vm-parser.cpp
```
