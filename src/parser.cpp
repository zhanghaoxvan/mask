#include "parser.hpp"
#include <algorithm>

ErrorReporter::Error ErrorReporter::report(int line, const std::string& message, const std::string& context) {
    errors.push_back({line, message, context});
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
    throw std::runtime_error(message);
}

void Parser::error(const Token& token, const std::string& message) {
    errorReporter.report(token.line, message, token.lexeme);
    throw std::runtime_error("Parse error at line " + std::to_string(token.line) + ": " + message);
}
