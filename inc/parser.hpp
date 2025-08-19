#pragma once

#include "inc.hpp"
#include "lexer.hpp"
#include "ast.hpp"

class ErrorReporter {
public:
    struct Error {
        int line;
        std::string message;
        std::string context;
    };
    
    Error report(int line, const std::string& message, const std::string& context = "");
    bool hasErrors() const;
    const std::vector<Error>& getErrors() const;
    void clear();
    
private:
    std::vector<Error> errors;
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens, ErrorReporter& errorReporter);
    std::unique_ptr<ASTTree> parse();
    
private:
    struct Scope {
        std::unordered_map<std::string, ASTNode*> symbols;
        std::string name;
    };
    
    void enterScope(const std::string& name = "");
    void exitScope();
    void declare(const std::string& name, ASTNode* node);
    ASTNode* resolve(const std::string& name);
    
    const Token& peek() const;
    const Token& advance();
    const Token& previous() const;
    bool isAtEnd() const;
    bool match(TokenType type);
    bool check(TokenType type) const;
    const Token& consume(TokenType type, const std::string& message);
    void sync();
    
    ASTNode::Ptr parseDeclaration();
    ASTNode::Ptr parseModuleDecl();
    ASTNode::Ptr parseFunctionDecl();
    ASTNode::Ptr parseStructDecl();      // todo: parseStructDecl实现
    ASTNode::Ptr parseInterfaceDecl();   // todo: parseInterfaceDecl实现
    ASTNode::Ptr parseImplDecl();        // todo: parseImplDecl实现
    ASTNode::Ptr parseImportDecl();      // todo: parseImportDecl实现
    ASTNode::Ptr parseConstructorDecl(); // todo: parseConstructorDecl实现
    ASTNode::Ptr parseOperatorDecl();    // todo: parseOperatorDecl实现
    ASTNode::Ptr parseParameters();
    ASTNode::Ptr parseParameter();
    ASTNode::Ptr parseBlock();
    ASTNode::Ptr parseStatement();
    ASTNode::Ptr parseIfStmt();
    ASTNode::Ptr parseWhileStmt();
    ASTNode::Ptr parseForStmt();         // todo: parseForStmt实现
    ASTNode::Ptr parseReturnStmt();
    ASTNode::Ptr parseVarDecl();
    ASTNode::Ptr parsePrintStmt();
    ASTNode::Ptr parseExpression();
    ASTNode::Ptr parseAssignment();
    ASTNode::Ptr parseLogicalOr();
    ASTNode::Ptr parseLogicalAnd();
    ASTNode::Ptr parseEquality();
    ASTNode::Ptr parseComparison();
    ASTNode::Ptr parseTerm();
    ASTNode::Ptr parseFactor();
    ASTNode::Ptr parseUnary();
    ASTNode::Ptr parsePrimary();
    ASTNode::Ptr parseCall();
    ASTNode::Ptr parseType();
    ASTNode::Ptr parseExprStmt();
    ASTNode::Ptr finishCall(ASTNode::Ptr callee);
    ASTNode::Ptr parseRangeExpr();
    ASTNode::Ptr parseInterpolationExpression(const std::string& exprText, int line);
    std::vector<ASTNode::Ptr> parseStringInterpolation(const Token& stringToken);
    
    void error(const Token& token, const std::string& message);
    int getPrecedence(TokenType type) const;
    
private:
    const std::vector<Token> tokens;
    size_t current = 0;
    ErrorReporter& errorReporter;
    std::vector<Scope> scopes;
    std::string currentModule;
};
