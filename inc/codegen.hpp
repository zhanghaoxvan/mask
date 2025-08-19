#pragma once

#include "ast.hpp"

class CodeGenerator {
public:
    CodeGenerator();
    
    void generate(const ASTTree& ast);
    
private:
    // LLVM 核心组件
    llvm::LLVMContext context;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;
private:
    
    // 语言特性支持
    struct StructInfo {
        llvm::StructType* type;
        std::map<std::string, int> memberIndices;
    };
    
    struct InterfaceImpl {
        llvm::Function* constructor;
        llvm::Function* strMethod;
        std::map<std::string, llvm::Function*> operators;
    };
    
    // 状态管理
    std::string currentModule;
    std::map<std::string, StructInfo> structs;
    std::map<std::string, InterfaceImpl> interfaces;
    std::map<std::string, llvm::Function*> functions;
    std::map<std::string, llvm::Value*> variables;
    std::map<std::string, std::string> imports;
    
    // 运行时函数声明
    llvm::Function* printStringFunc;
    llvm::Function* intToStringFunc;
    llvm::Function* stringConcatFunc;
    
    // 核心生成方法
    void generateDeclaration(const ASTNode* node);
    void generateModule(const ASTNode* node);
    void generateImport(const ASTNode* node);
    void generateFunction(const ASTNode* node);
    void generateStruct(const ASTNode* node);
    void generateInterfaceImpl(const ASTNode* node);
    void generateConstructor(const ASTNode* node, const std::string& structName);
    void generateStrMethod(const ASTNode* node, const std::string& structName);
    void generateOperator(const ASTNode* node, const std::string& structName, const std::string& op);
    void generateMainFunction(const ASTNode* node);
    
    // 表达式生成
    llvm::Value* generateExpression(const ASTNode* node);
    llvm::Value* generateStringInterpolation(const ASTNode* node);
    llvm::Value* generateCall(const ASTNode* node);
    llvm::Value* generateMemberAccess(const ASTNode* node);
    llvm::Value* generateStructInitialization(const ASTNode* node);
    llvm::Value* generateLiteral(const ASTNode* node);
    llvm::Value* generateBinaryOp(const ASTNode* node);
    
    // 辅助方法
    llvm::Type* mapType(const std::string& typeName);
    llvm::Value* getMemberPointer(llvm::Value* structPtr, const std::string& memberName);
    void declareRuntimeFunctions();
};
