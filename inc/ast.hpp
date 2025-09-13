#pragma once

#include "inc.hpp"
#include "lexer.hpp"

enum class NodeType {
    // Top声明
    TopDecl,

    // 声明类型
    ModuleDecl,
    FunctionDecl,
    StructDecl,
    InterfaceDecl,
    ImplDecl,
    ImportDecl,
    ConstructorDecl,
    OperatorDecl,
    VarDecl,
    
    // 语句类型
    ReturnStmt,
    PrintStmt,
    ExprStmt,
    BlockStmt,
    IfStmt,
    WhileStmt,
    ForStmt,
    
    // 表达式类型
    BinaryExpr,
    UnaryExpr,
    CallExpr,
    MemberAccess,
    Identifier,
    Literal,
    GroupExpr,
    Parameter,
    TypeRef,
    StringInterpolation,
    InterpolationText,
    ArrayAccess,
    ThisExpr,
    RangeExpr
};

struct TypeInfo {
    std::string name;
    bool isBuiltin = false;
    bool isGeneric = false;
    std::vector<TypeInfo> genericParams;
};

struct ASTNode {
    using Ptr = std::unique_ptr<ASTNode>;
    using ParentPtr = std::shared_ptr<ASTNode>;
    ParentPtr parent = nullptr;
    std::vector<Ptr> children;
    Token token;
    NodeType type;
    TypeInfo typeInfo;
    
    ASTNode(Token t, NodeType nt) : token(t), type(nt) {}
    ASTNode(Token t, NodeType nt, std::vector<Ptr>&& c);
    ~ASTNode() = default;
};

struct ASTTree {
    ASTNode::Ptr root;
    explicit ASTTree(ASTNode::Ptr r = nullptr) : root(std::move(r)) {}
    operator ASTNode::Ptr() {
        return std::move(root);
    }
    void setRoot(ASTNode::Ptr node);
    ASTNode* getRoot() const;
    void clear();
    bool isEmpty() const;
    
    void traversePreOrder(ASTNode* node, const std::function<void(ASTNode*)>& func);
    void traverseLevelOrder(const std::function<void(ASTNode*)>& func);
};
