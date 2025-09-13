#include <iostream>
#include "lexer.hpp"
#include "ast.hpp"
#include "parser.hpp"

#define ParseArgsToVector(argc, argv, vectorName) \
    std::vector<std::string> vectorName(argv, argv + argc)

std::string readFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filePath);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
std::vector<std::string>::iterator getFileName(std::vector<std::string> commands) {
    // 找第一个前面没有/或者-或者--的
    return std::find(commands.begin(), commands.end(), [](const std::string& cmd) {
        return cmd[0] != '-' && cmd[0] != '/';
    });
}
std::unordered_map<std::string, std::function<void()>> commands;
int main(int argc, char* argv[]) {
    ParseArgsToVector(argc, argv, args);
    auto fileNameIter = getFileName(args);
    Lexer lexer(readFile(*fileNameIter));
    auto tokens = lexer.scanTokens();
    ErrorReporter errRep;
    Parser parser(tokens, errRep);
    ASTTree tree = parser.parse();
    if (errRep.hasErrors()) {
        auto errs = errRep.getErrors();
        for (const auto& err : errs) {
            std::cerr << "[Error at " << err.line << "] " << err.message;
        }
    }
    
    return 0;
}
