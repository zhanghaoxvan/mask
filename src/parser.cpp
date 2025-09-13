#include "parser.hpp"
#include <algorithm>

ErrorReporter::Error ErrorReporter::report(int line, const std::string& message) {
    errors.push_back({line, message});
    return errors.back();
}

bool ErrorReporter::hasErrors() const {
    return !errors.empty();
}

const std::vector<ErrorReporter::Error>& ErrorReporter::getErrors() const {
    return errors;
}

void ErrorReporter::clear() {
    errors.clear();
}

Parser::Parser(std::vector<Token> tokens, ErrorReporter& errorReporter)
    : tokens(std::move(tokens)), errorReporter(errorReporter) {
    enterScope("global");
}

ASTTree Parser::parse() {
    auto root = parseTopDeclaration();
    return ASTTree(std::move(root));
}

void Parser::enterScope(const std::string& name) {
    scopes.push_back({ {}, name });
}

void Parser::exitScope() {
    if (!scopes.empty()) {
        scopes.pop_back();
    }
}

void Parser::declare(const std::string& name, ASTNode* node) {
    if (scopes.empty()) return;
    
    auto& currentScope = scopes.back();
    if (currentScope.symbols.find(name) != currentScope.symbols.end()) {
        error(previous(), "Duplicate identifier '" + name + "' in scope '" + currentScope.name + "'");
    }
    currentScope.symbols[name] = node;
}

ASTNode* Parser::resolve(const std::string& name) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto symbolIt = it->symbols.find(name);
        if (symbolIt != it->symbols.end()) {
            return symbolIt->second;
        }
    }
    return nullptr;
}

const Token& Parser::peek() const {
    return tokens[current];
}

const Token& Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

const Token& Parser::previous() const {
    return tokens[current - 1];
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::Eof;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

const Token& Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    
    error(peek(), message);
    // throw std::runtime_error(message);
}

void Parser::error(const Token& token, const std::string& message) {
    errorReporter.report(token.line, message);
    // 不throw
    // throw std::runtime_error("Parse error at line " + std::to_string(token.line) + ": " + message); 
}

ASTNode::Ptr Parser::parseTopDeclaration() {
    // 顶级声明：module声明，import导入，function声明，struct声明，interface声明，impl声明，constructor声明，operator声明，变量声明

    std::vector<ASTNode::Ptr> decls;
    if (match(TokenType::KwModule)) {
        decls.push_back(parseModuleDeclaration());
    } else if (match(TokenType::KwImport)) {
        decls.push_back(parseImportDeclaration());
    }
    return std::make_unique<ASTNode>(new ASTNode(
        Token(TokenType::Top, "", 0), // 总结点没有行数，记作0
        NodeType::TopDecl,
        std::move(decls)
    ));
}

ASTNode::Ptr Parser::parseModuleDeclaration() {
    // module moduleName
    if (peek().type != TokenType::Identifier) {
        error(peek(), "Expected module name after 'module'");
        if (peek().line != 1) {
            error(peek(), "Module declaration must be at the top of the file");   
        }
        return std::make_unique<ASTNode>(new ASTNode(
            Token(TokenType::KwModule, "module", peek().line),
            NodeType::ModuleDecl
        ));
    }
}
