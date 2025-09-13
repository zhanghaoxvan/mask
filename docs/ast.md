# AST（抽象语法树）文档

## 节点类型 (NodeType)

### 声明类型
- **ModuleDecl**: 模块声明
- **FunctionDecl**: 函数声明
- **StructDecl**: 结构体声明
- **InterfaceDecl**: 接口声明
- **ImplDecl**: 实现声明
- **ImportDecl**: 导入声明
- **ConstructorDecl**: 构造函数声明
- **OperatorDecl**: 运算符声明
- **VarDecl**: 变量声明

### 语句类型
- **ReturnStmt**: 返回语句
- **PrintStmt**: 打印语句
- **ExprStmt**: 表达式语句
- **BlockStmt**: 块语句
- **IfStmt**: 条件语句
- **WhileStmt**: while循环语句
- **ForStmt**: for循环语句

### 表达式类型
- **BinaryExpr**: 二元表达式
- **UnaryExpr**: 一元表达式
- **CallExpr**: 函数调用表达式
- **MemberAccess**: 成员访问表达式
- **Identifier**: 标识符
- **Literal**: 字面量
- **GroupExpr**: 分组表达式
- **Parameter**: 参数
- **TypeRef**: 类型引用
- **StringInterpolation**: 字符串插值
- **InterpolationText**: 插值文本
- **ArrayAccess**: 数组访问
- **ThisExpr**: this表达式
- **RangeExpr**: 范围表达式

## 类型信息 (TypeInfo)

### `struct TypeInfo`
- **描述**: 表示类型的相关信息
- **成员**:
  - `std::string name`: 类型名称
  - `bool isBuiltin`: 是否为内置类型
  - `bool isGeneric`: 是否为泛型类型
  - `std::vector<TypeInfo> genericParams`: 泛型参数列表

## 抽象语法树节点 (ASTNode)

### `using Ptr`
- **描述**: 一个ASTNode的unique pointer

### `using ParentPtr`
- **描述**: 一个ASTNode的shared pointer

### `struct ASTNode`
- **描述**: 表示抽象语法树中的一个节点
- **成员**:
  - `ParentPtr parent`: 父节点指针
  - `std::vector<Ptr> children`: 子节点列表
  - `Token token`: 与节点关联的词法单元
  - `NodeType type`: 节点类型
  - `TypeInfo typeInfo`: 节点的类型信息

### `ASTNode(Token t, NodeType nt)`
- **描述**: 构造函数，创建一个没有子节点的AST节点
- **参数**:
  - `t`: 与节点关联的词法单元
  - `nt`: 节点类型

### `ASTNode(Token t, NodeType nt, std::vector<Ptr>&& c)`
- **描述**: 构造函数，创建一个带有子节点的AST节点
- **参数**:
  - `t`: 与节点关联的词法单元
  - `nt`: 节点类型
  - `c`: 子节点列表

## 抽象语法树 (ASTTree)

### `struct ASTTree`
- **描述**: 表示完整的抽象语法树
- **成员**:
  - `ASTNode::Ptr root`: 根节点

### `operator ASTNode::Ptr()`
- **描述**: 类型转换运算符，将ASTTree转换为ASTNode::Ptr
- **返回值**: 返回根节点的所有权

### `void setRoot(ASTNode::Ptr node)`
- **描述**: 设置抽象语法树的根节点
- **参数**:
  - `node`: 新的根节点

### `ASTNode* getRoot() const`
- **描述**: 获取抽象语法树的根节点
- **返回值**: 返回根节点的指针

### `void clear()`
- **描述**: 清空抽象语法树

### `bool isEmpty() const`
- **描述**: 检查抽象语法树是否为空
- **返回值**: 如果树为空返回true，否则返回false

### `void traversePreOrder(ASTNode* node, const std::function<void(ASTNode*)>& func)`
- **描述**: 前序遍历抽象语法树
- **参数**:
  - `node`: 开始遍历的节点
  - `func`: 对每个节点执行的函数

### `void traverseLevelOrder(const std::function<void(ASTNode*)>& func)`
- **描述**: 层序遍历抽象语法树
- **参数**:
  - `func`: 对每个节点执行的函数
