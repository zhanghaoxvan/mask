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

std::unique_ptr<ASTTree> Parser::parse() {
    auto tree = std::make_unique<ASTTree>();
    std::vector<ASTNode::Ptr> topLevelNodes;
    
    while (!isAtEnd()) {
        try {
            auto decl = parseDeclaration();
            if (decl) {
                topLevelNodes.push_back(std::move(decl));
            }
        } catch (const std::exception& e) {
            errorReporter.report(peek().line, "Parse error: " + std::string(e.what()));
            sync();
        }
    }
    
    if (!topLevelNodes.empty()) {
        tree->setRoot(std::make_unique<ASTNode>(
            Token(TokenType::Eof, "", 0),
            NodeType::BlockStmt,
            std::move(topLevelNodes)
        ));
    }
    
    return tree;
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

void Parser::sync() {
    advance();
    
    while (!isAtEnd()) {
        if (previous().type == TokenType::Semicolon) return;
        
        switch (peek().type) {
            case TokenType::KwFunc:
            case TokenType::KwModule:
            case TokenType::KwStruct:
            case TokenType::KwInterface:
            case TokenType::KwImpl:
            case TokenType::KwImport:
            case TokenType::KwIf:
            case TokenType::KwWhile:
            case TokenType::KwFor:
            case TokenType::KwReturn:
                return;
            default:
                advance();
        }
    }
}

int Parser::getPrecedence(TokenType type) const {
    static const std::unordered_map<TokenType, int> precedences = {
        {TokenType::Assign, 10},
        
        {TokenType::Or, 20},
        {TokenType::And, 30},
        
        {TokenType::Equal, 40},
        {TokenType::NotEqual, 40},
        {TokenType::Less, 50},
        {TokenType::LessEqual, 50},
        {TokenType::Greater, 50},
        {TokenType::GreaterEqual, 50},
        
        {TokenType::Plus, 60},
        {TokenType::Minus, 60},
        
        {TokenType::Star, 70},
        {TokenType::Slash, 70},
        
        {TokenType::Not, 80},
        {TokenType::Dot, 90},
        {TokenType::LBracket, 90},
        {TokenType::LParen, 90}
    };
    
    auto it = precedences.find(type);
    return it != precedences.end() ? it->second : 0;
}

ASTNode::Ptr Parser::parseDeclaration() {
    if (match(TokenType::KwModule)) return parseModuleDecl();
    if (match(TokenType::KwFunc)) return parseFunctionDecl();
    if (match(TokenType::KwImport)) return parseImportDecl();
    if (match(TokenType::KwStruct)) return parseStructDecl();
    if (match(TokenType::KwInterface)) return parseInterfaceDecl();
    if (match(TokenType::KwImpl)) return parseImplDecl();
    
    return parseStatement();
}

ASTNode::Ptr Parser::parseModuleDecl() {
    const Token& nameToken = consume(TokenType::Identifier, "Expected module name after 'module'");
    currentModule = nameToken.lexeme;
    
    enterScope(currentModule);
    
    std::vector<ASTNode::Ptr> body;
    consume(TokenType::LBrace, "Expected '{' after module name");
    while (!check(TokenType::RBrace) && !isAtEnd()) {
        body.push_back(parseDeclaration());
    }
    consume(TokenType::RBrace, "Expected '}' after module body");
    
    exitScope();
    
    auto moduleNode = std::make_unique<ASTNode>(
        Token(TokenType::KwModule, "module", nameToken.line),
        NodeType::ModuleDecl
    );
    
    moduleNode->children.push_back(std::make_unique<ASTNode>(nameToken, NodeType::Identifier));
    moduleNode->children.push_back(std::make_unique<ASTNode>(
        Token(TokenType::LBrace, "{", nameToken.line),
        NodeType::BlockStmt,
        std::move(body)
    ));
    
    return moduleNode;
}

ASTNode::Ptr Parser::parseFunctionDecl() {
    const Token& funcToken = previous();
    const Token& nameToken = consume(TokenType::Identifier, "Expected function name after 'func'");
    
    auto funcNode = std::make_unique<ASTNode>(funcToken, NodeType::FunctionDecl);
    declare(nameToken.lexeme, funcNode.get());
    
    enterScope("func:" + nameToken.lexeme);
    
    auto paramsNode = parseParameters();
    ASTNode::Ptr retTypeNode = nullptr;
    
    if (match(TokenType::ReturnType)) {
        retTypeNode = parseType();
    }
    
    auto bodyNode = parseBlock();
    exitScope();
    
    funcNode->children.push_back(std::make_unique<ASTNode>(nameToken, NodeType::Identifier));
    funcNode->children.push_back(std::move(paramsNode));
    if (retTypeNode) funcNode->children.push_back(std::move(retTypeNode));
    funcNode->children.push_back(std::move(bodyNode));
    
    return funcNode;
}

ASTNode::Ptr Parser::parseStructDecl() {
    
}

ASTNode::Ptr Parser::parseParameters() {
    auto paramsNode = std::make_unique<ASTNode>(
        Token(TokenType::LParen, "(", peek().line),
        NodeType::BlockStmt
    );
    
    consume(TokenType::LParen, "Expected '(' after function name");
    if (!check(TokenType::RParen)) {
        do {
            paramsNode->children.push_back(parseParameter());
        } while (match(TokenType::Comma));
    }
    consume(TokenType::RParen, "Expected ')' after parameters");
    
    return paramsNode;
}

ASTNode::Ptr Parser::parseParameter() {
    const Token& nameToken = consume(TokenType::Identifier, "Expected parameter name");
    consume(TokenType::Colon, "Expected ':' after parameter name");
    auto typeNode = parseType();
    
    auto paramNode = std::make_unique<ASTNode>(nameToken, NodeType::Parameter);
    declare(nameToken.lexeme, paramNode.get());
    
    paramNode->children.push_back(std::move(typeNode));
    return paramNode;
}

ASTNode::Ptr Parser::parseType() {
    if (match(TokenType::Identifier)) {
        const Token& typeToken = previous();
        auto typeNode = std::make_unique<ASTNode>(typeToken, NodeType::TypeRef);
        typeNode->typeInfo.name = typeToken.lexeme;
        typeNode->typeInfo.isBuiltin = (typeToken.lexeme == "int" || 
                                       typeToken.lexeme == "float" || 
                                       typeToken.lexeme == "string" || 
                                       typeToken.lexeme == "bool");
        
        if (match(TokenType::Less)) {
            typeNode->typeInfo.isGeneric = true;
            while (!check(TokenType::Greater) && !isAtEnd()) {
                typeNode->children.push_back(parseType());
                if (!match(TokenType::Comma)) break;
            }
            consume(TokenType::Greater, "Expected '>' after type parameters");
        }
        
        return typeNode;
    }
    
    error(peek(), "Expected type identifier");
    return nullptr;
}

ASTNode::Ptr Parser::parseBlock() {
    const Token& lbrace = consume(TokenType::LBrace, "Expected '{'");
    enterScope("block");
    
    std::vector<ASTNode::Ptr> statements;
    while (!check(TokenType::RBrace) && !isAtEnd()) {
        statements.push_back(parseStatement());
    }
    
    consume(TokenType::RBrace, "Expected '}' after block");
    exitScope();
    
    return std::make_unique<ASTNode>(
        lbrace,
        NodeType::BlockStmt,
        std::move(statements)
    );
}

ASTNode::Ptr Parser::parseStatement() {
    if (match(TokenType::KwIf)) return parseIfStmt();
    if (match(TokenType::KwWhile)) return parseWhileStmt();
    if (match(TokenType::KwFor)) return parseForStmt();
    if (match(TokenType::KwReturn)) return parseReturnStmt();
    if (match(TokenType::KwVar)) return parseVarDecl();
    if (check(TokenType::Identifier) && peek().lexeme == "print") {
        advance();
        return parsePrintStmt();
    }
    if (check(TokenType::Semicolon)) {
        advance();
        return std::make_unique<ASTNode>(
            previous(),
            NodeType::ExprStmt
        );
    }
    return parseExprStmt();
}

ASTNode::Ptr Parser::parseIfStmt() {
    const Token& ifToken = previous();
    auto condition = parseExpression();
    
    auto thenBranch = parseStatement();
    ASTNode::Ptr elseBranch = nullptr;
    
    if (match(TokenType::KwElse)) {
        elseBranch = parseStatement();
    }
    
    auto ifNode = std::make_unique<ASTNode>(ifToken, NodeType::IfStmt);
    ifNode->children.push_back(std::move(condition));
    ifNode->children.push_back(std::move(thenBranch));
    if (elseBranch) ifNode->children.push_back(std::move(elseBranch));
    
    return ifNode;
}

ASTNode::Ptr Parser::parseWhileStmt() {
    const Token& whileToken = previous();
    auto condition = parseExpression();
    
    auto body = parseStatement();
    
    auto whileNode = std::make_unique<ASTNode>(whileToken, NodeType::WhileStmt);
    whileNode->children.push_back(std::move(condition));
    whileNode->children.push_back(std::move(body));
    
    return whileNode;
}

ASTNode::Ptr Parser::parseRangeExpr() {
    auto start = parseExpression();
    consume(TokenType::Tilde, "Expected '~' in range expression");
    auto end = parseExpression();
    
    auto rangeNode = std::make_unique<ASTNode>(
        Token(TokenType::Tilde, "~", start->token.line),
        NodeType::RangeExpr
    );
    rangeNode->children.push_back(std::move(start));
    rangeNode->children.push_back(std::move(end));
    
    // 处理步长
    if (match(TokenType::Comma)) {
        auto step = parseExpression();
        rangeNode->children.push_back(std::move(step));
    } else {
        // 默认步长为1
        rangeNode->children.push_back(std::make_unique<ASTNode>(
            Token(TokenType::Number, "1", end->token.line),
            NodeType::Literal
        ));
    }
    
    return rangeNode;
}

ASTNode::Ptr Parser::parseReturnStmt() {
    const Token& returnToken = previous();
    ASTNode::Ptr expr = nullptr;
    
    if (!check(TokenType::Semicolon)) {
        expr = parseExpression();
    }
    
    auto returnNode = std::make_unique<ASTNode>(returnToken, NodeType::ReturnStmt);
    if (expr) returnNode->children.push_back(std::move(expr));
    return returnNode;
}

ASTNode::Ptr Parser::parseVarDecl() {
    const Token& varToken = previous();
    const Token& nameToken = consume(TokenType::Identifier, "Expected variable name after 'var'");
    consume(TokenType::Colon, "Expected ':' after variable name");
    auto typeNode = parseType();
    consume(TokenType::Assign, "Expected '=' after type");
    auto initializer = parseExpression();
    
    auto varNode = std::make_unique<ASTNode>(varToken, NodeType::VarDecl);
    declare(nameToken.lexeme, varNode.get());
    
    varNode->children.push_back(std::make_unique<ASTNode>(nameToken, NodeType::Identifier));
    varNode->children.push_back(std::move(typeNode));
    varNode->children.push_back(std::move(initializer));
    
    return varNode;
}

ASTNode::Ptr Parser::parsePrintStmt() {
    const Token& printToken = previous();
    consume(TokenType::LParen, "Expected '(' after 'print'");
    auto expr = parseExpression();
    consume(TokenType::RParen, "Expected ')' after expression");
    
    auto printNode = std::make_unique<ASTNode>(printToken, NodeType::PrintStmt);
    printNode->children.push_back(std::move(expr));
    return printNode;
}

ASTNode::Ptr Parser::parseExprStmt() {
    auto expr = parseExpression();
    consume(TokenType::Semicolon, "Expected ';' after expression");
    
    auto exprStmt = std::make_unique<ASTNode>(
        Token(TokenType::Semicolon, ";", expr->token.line),
        NodeType::ExprStmt
    );
    exprStmt->children.push_back(std::move(expr));
    return exprStmt;
}

ASTNode::Ptr Parser::parseExpression() {
    return parseAssignment();
}

ASTNode::Ptr Parser::parseAssignment() {
    auto expr = parseLogicalOr();
    
    if (match(TokenType::Assign)) {
        const Token& equals = previous();
        auto value = parseAssignment();
        
        if (expr->type == NodeType::Identifier || 
            expr->type == NodeType::MemberAccess || 
            expr->type == NodeType::ArrayAccess) {
            auto assignNode = std::make_unique<ASTNode>(equals, NodeType::BinaryExpr);
            assignNode->children.push_back(std::move(expr));
            assignNode->children.push_back(std::move(value));
            return assignNode;
        }
        
        error(equals, "Invalid assignment target");
    }
    
    return expr;
}

ASTNode::Ptr Parser::parseLogicalOr() {
    auto expr = parseLogicalAnd();
    
    while (match(TokenType::Or)) {
        const Token& op = previous();
        auto right = parseLogicalAnd();
        auto logicalNode = std::make_unique<ASTNode>(op, NodeType::BinaryExpr);
        logicalNode->children.push_back(std::move(expr));
        logicalNode->children.push_back(std::move(right));
        expr = std::move(logicalNode);
    }
    
    return expr;
}

ASTNode::Ptr Parser::parseLogicalAnd() {
    auto expr = parseEquality();
    
    while (match(TokenType::And)) {
        const Token& op = previous();
        auto right = parseEquality();
        auto logicalNode = std::make_unique<ASTNode>(op, NodeType::BinaryExpr);
        logicalNode->children.push_back(std::move(expr));
        logicalNode->children.push_back(std::move(right));
        expr = std::move(logicalNode);
    }
    
    return expr;
}

ASTNode::Ptr Parser::parseEquality() {
    auto expr = parseComparison();
    
    while (match(TokenType::Equal) || match(TokenType::NotEqual)) {
        const Token& op = previous();
        auto right = parseComparison();
        auto equalityNode = std::make_unique<ASTNode>(op, NodeType::BinaryExpr);
        equalityNode->children.push_back(std::move(expr));
        equalityNode->children.push_back(std::move(right));
        expr = std::move(equalityNode);
    }
    
    return expr;
}

ASTNode::Ptr Parser::parseComparison() {
    auto expr = parseTerm();
    
    while (match(TokenType::Less) || match(TokenType::LessEqual) || 
           match(TokenType::Greater) || match(TokenType::GreaterEqual)) {
        const Token& op = previous();
        auto right = parseTerm();
        auto compNode = std::make_unique<ASTNode>(op, NodeType::BinaryExpr);
        compNode->children.push_back(std::move(expr));
        compNode->children.push_back(std::move(right));
        expr = std::move(compNode);
    }
    
    return expr;
}

ASTNode::Ptr Parser::parseTerm() {
    auto expr = parseFactor();
    
    while (match(TokenType::Plus) || match(TokenType::Minus)) {
        const Token& op = previous();
        auto right = parseFactor();
        auto termNode = std::make_unique<ASTNode>(op, NodeType::BinaryExpr);
        termNode->children.push_back(std::move(expr));
        termNode->children.push_back(std::move(right));
        expr = std::move(termNode);
    }
    
    return expr;
}

ASTNode::Ptr Parser::parseFactor() {
    auto expr = parseUnary();
    
    while (match(TokenType::Star) || match(TokenType::Slash)) {
        const Token& op = previous();
        auto right = parseUnary();
        auto factorNode = std::make_unique<ASTNode>(op, NodeType::BinaryExpr);
        factorNode->children.push_back(std::move(expr));
        factorNode->children.push_back(std::move(right));
        expr = std::move(factorNode);
    }
    
    return expr;
}

ASTNode::Ptr Parser::parseUnary() {
    if (match(TokenType::Not) || match(TokenType::Minus)) {
        const Token& op = previous();
        auto right = parseUnary();
        auto unaryNode = std::make_unique<ASTNode>(op, NodeType::UnaryExpr);
        unaryNode->children.push_back(std::move(right));
        return unaryNode;
    }
    
    return parseCall();
}

ASTNode::Ptr Parser::parseCall() {
    auto expr = parsePrimary();
    
    while (true) {
        if (match(TokenType::LParen)) {
            expr = finishCall(std::move(expr));
        } else if (match(TokenType::Dot)) {
            const Token& name = consume(TokenType::Identifier, "Expected property name after '.'");
            auto accessNode = std::make_unique<ASTNode>(name, NodeType::MemberAccess);
            accessNode->children.push_back(std::move(expr));
            accessNode->children.push_back(std::make_unique<ASTNode>(name, NodeType::Identifier));
            expr = std::move(accessNode);
        } else if (match(TokenType::LBracket)) {
            auto index = parseExpression();
            consume(TokenType::RBracket, "Expected ']' after index");
            auto accessNode = std::make_unique<ASTNode>(previous(), NodeType::ArrayAccess);
            accessNode->children.push_back(std::move(expr));
            accessNode->children.push_back(std::move(index));
            expr = std::move(accessNode);
        } else {
            break;
        }
    }
    
    return expr;
}

ASTNode::Ptr Parser::finishCall(ASTNode::Ptr callee) {
    std::vector<ASTNode::Ptr> args;
    if (!check(TokenType::RParen)) {
        do {
            args.push_back(parseExpression());
        } while (match(TokenType::Comma));
    }
    
    const Token& paren = consume(TokenType::RParen, "Expected ')' after arguments");
    
    auto callNode = std::make_unique<ASTNode>(paren, NodeType::CallExpr);
    callNode->children.push_back(std::move(callee));
    for (auto& arg : args) {
        callNode->children.push_back(std::move(arg));
    }
    
    return callNode;
}

ASTNode::Ptr Parser::parsePrimary() {
    if (match(TokenType::KwTrue)) {
        return std::make_unique<ASTNode>(previous(), NodeType::Literal);
    }
    if (match(TokenType::KwFalse)) {
        return std::make_unique<ASTNode>(previous(), NodeType::Literal);
    }
    if (match(TokenType::KwThis)) {
        return std::make_unique<ASTNode>(previous(), NodeType::ThisExpr);
    }
    if (match(TokenType::Number) || match(TokenType::Str)) {
        return std::make_unique<ASTNode>(previous(), NodeType::Literal);
    }
    if (match(TokenType::Identifier)) {
        return std::make_unique<ASTNode>(previous(), NodeType::Identifier);
    }
    if (match(TokenType::LParen)) {
        auto expr = parseExpression();
        consume(TokenType::RParen, "Expected ')' after expression");
        return std::make_unique<ASTNode>(
            previous(),
            NodeType::GroupExpr,
            std::vector<ASTNode::Ptr>{std::move(expr)}
        );
    }

    if (match(TokenType::Str)) {
        const Token& stringToken = previous();
        // 解析字符串中的插值表达式
        auto parts = parseStringInterpolation(stringToken);
        // 创建字符串插值根节点
        auto interpolationNode = std::make_unique<ASTNode>(
            stringToken,
            NodeType::StringInterpolation
        );
        // 添加所有解析出的部分（文本或表达式）
        for (auto& part : parts) {
            interpolationNode->children.push_back(std::move(part));
        }
        return interpolationNode;
    }
    
    error(peek(), "Expected expression");
    return nullptr;
}

std::vector<ASTNode::Ptr> Parser::parseStringInterpolation(const Token& stringToken) {
    std::vector<ASTNode::Ptr> parts;
    const std::string& str = stringToken.lexeme;
    size_t pos = 0;
    const size_t length = str.size();

    while (pos < length) {
        // 查找下一个 '{' 作为插值开始
        size_t bracePos = str.find('{', pos);
        
        if (bracePos > 0 && str[bracePos - 1] == '\\') {
            // 是转义的 '{', 不作为插值开始，添加普通 '{' 字符
            if (bracePos > pos) {
                std::string text = str.substr(pos, bracePos - pos - 1) + "{";
                parts.push_back(std::make_unique<ASTNode>(
                    Token(TokenType::Str, text, stringToken.line),
                    NodeType::InterpolationText
                ));
            }
            pos = bracePos + 1;
            continue;
        }

        if (bracePos == std::string::npos) {
            // 没有更多插值，添加剩余文本
            std::string text = str.substr(pos);
            parts.push_back(std::make_unique<ASTNode>(
                Token(TokenType::Str, text, stringToken.line),
                NodeType::InterpolationText
            ));
            break;
        }

        // 添加 '{' 之前的普通文本
        if (bracePos > pos) {
            std::string text = str.substr(pos, bracePos - pos);
            parts.push_back(std::make_unique<ASTNode>(
                Token(TokenType::Str, text, stringToken.line),
                NodeType::InterpolationText
            ));
        }

        // 解析插值表达式（跳过 '{'）
        pos = bracePos + 1;
        // 查找对应的 '}'（不支持嵌套）
        size_t endPos = str.find('}', pos);
        if (endPos == std::string::npos) {
            // 错误：未闭合的 '{'
            error(stringToken, "Unclosed '{' in string interpolation");
            // 错误恢复：将剩余部分作为普通文本
            parts.push_back(std::make_unique<ASTNode>(
                Token(TokenType::Str, str.substr(bracePos), stringToken.line),
                NodeType::InterpolationText
            ));
            break;
        }

        // 提取表达式文本（如 "a + b"）
        std::string exprText = str.substr(pos, endPos - pos);
        pos = endPos + 1;  // 跳过 '}'

        // 解析表达式并添加到结果
        auto exprNode = parseInterpolationExpression(exprText, stringToken.line);
        if (exprNode) {
            parts.push_back(std::move(exprNode));
        } else {
            // 表达式解析失败，添加错误标记（使用现有节点类型）
            parts.push_back(std::make_unique<ASTNode>(
                Token(TokenType::Error, "Invalid expression: " + exprText, stringToken.line),
                NodeType::Literal
            ));
        }
    }

    return parts;
}
