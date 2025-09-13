# Parser类函数文档

## ErrorReporter类

### `Error report(int line, const std::string& message)`
- **描述**：报告一个解析错误
- **参数**：
  - `line`：错误发生的行号
  - `message`：错误信息
- **返回值**：返回创建的Error对象

### `bool hasErrors() const`
- **描述**：检查是否有解析错误
- **返回值**：如果有错误返回true，否则返回false

### `const std::vector<Error>& getErrors() const`
- **描述**：获取所有解析错误
- **返回值**：返回错误列表的常量引用

### `void clear()`
- **描述**：清除所有错误记录

## Parser类

### `Parser(std::vector<Token> tokens, ErrorReporter& errorReporter)`
- **描述**：构造函数，初始化解析器
- **参数**：
  - `tokens`：词法分析器生成的token列表
  - `errorReporter`：错误报告器的引用

### `ASTTree parse()`
- **描述**：解析源代码，生成抽象语法树
- **返回值**：返回构建的抽象语法树

### `void enterScope(const std::string& name)`
- **描述**：进入一个新的作用域
- **参数**：
  - `name`：作用域的名称，默认为空字符串

### `void exitScope()`
- **描述**：退出当前作用域

### `void declare(const std::string& name, ASTNode* node)`
- **描述**：在当前作用域中声明一个标识符
- **参数**：
  - `name`：标识符名称
  - `node`：与标识符关联的AST节点

### `ASTNode* resolve(const std::string& name)`
- **描述**：解析一个标识符，查找其定义
- **参数**：
  - `name`：要解析的标识符名称
- **返回值**：如果找到标识符，返回关联的AST节点；否则返回nullptr

### `const Token& peek() const`
- **描述**：查看当前token，不消耗它
- **返回值**：返回当前token的常量引用

### `const Token& advance()`
- **描述**：前进到下一个token
- **返回值**：返回前一个token的常量引用

### `const Token& previous() const`
- **描述**：获取前一个token
- **返回值**：返回前一个token的常量引用

### `bool isAtEnd() const`
- **描述**：检查是否已到达token列表的末尾
- **返回值**：如果到达末尾返回true，否则返回false

### `bool match(TokenType type)`
- **描述**：检查当前token是否匹配指定类型，如果匹配则消耗它
- **参数**：
  - `type`：要匹配的token类型
- **返回值**：如果匹配成功返回true，否则返回false

### `bool check(TokenType type) const`
- **描述**：检查当前token是否匹配指定类型，不消耗它
- **参数**：
  - `type`：要检查的token类型
- **返回值**：如果类型匹配返回true，否则返回false

### `const Token& consume(TokenType type, const std::string& message)`
- **描述**：消耗当前token，如果类型不匹配则报告错误
- **参数**：
  - `type`：期望的token类型
  - `message`：错误信息
- **返回值**：返回消耗的token的常量引用

### `void sync()`
- **描述**：错误恢复，同步到下一个语句开始

### `void error(const Token& token, const std::string& message)`
- **描述**：报告解析错误
- **参数**：
  - `token`：错误相关的token
  - `message`：错误信息

## 内部数据结构

### `struct Scope`
- **描述**：表示一个作用域
- **成员**：
  - `std::unordered_map<std::string, ASTNode*> symbols`：作用域中的符号表
  - `std::string name`：作用域名称

## 私有成员变量

- `const std::vector<Token> tokens`：要解析的token列表
- `size_t current = 0`：当前token的索引
- `ErrorReporter& errorReporter`：错误报告器的引用
- `std::vector<Scope> scopes`：作用域栈
- `std::string currentModule`：当前模块名称
        