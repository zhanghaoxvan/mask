#include "mask-vm-parser.hpp"

// 辅助函数：分割字符串
std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        // 去除前后空格
        token.erase(token.begin(), std::find_if(token.begin(), token.end(), [](int ch) {
            return !std::isspace(ch);
        }));
        token.erase(std::find_if(token.rbegin(), token.rend(), [](int ch) {
            return !std::isspace(ch);
        }).base(), token.end());
        
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

// 解析数据类型
DataType MaskVM::parseDataType(const std::string& typeStr) {
    if (typeStr == "int") return DataType::INT;
    if (typeStr == "str") return DataType::STR;
    return DataType::UNKNOWN;
}

// 解析值
Value MaskVM::parseValue(DataType type, const std::string& valueStr) {
    if (type == DataType::INT) {
        try {
            return Value(std::stoi(valueStr));
        } catch (...) {
            lastError = "Invalid integer value: " + valueStr;
            return Value();
        }
    } else if (type == DataType::STR) {
        // 去除字符串的引号
        std::string str = valueStr;
        if (str.front() == '"' && str.back() == '"') {
            str = str.substr(1, str.size() - 2);
        }
        return Value(str);
    }
    lastError = "Unknown data type";
    return Value();
}

// 解析条件表达式
bool MaskVM::evaluateCondition(const std::string& condition) {
    // 简单处理 "a == int 5" 这样的表达式
    std::vector<std::string> parts = split(condition, ' ');
    if (parts.size() != 4 || parts[2] != "==") {
        lastError = "Invalid condition syntax: " + condition;
        return false;
    }
    
    std::string varName = parts[0];
    DataType type = parseDataType(parts[1]);
    if (type == DataType::UNKNOWN) {
        lastError = "Unknown type in condition: " + parts[1];
        return false;
    }
    
    // 检查变量是否存在
    auto it = variables.find(varName);
    if (it == variables.end()) {
        lastError = "Variable not found: " + varName;
        return false;
    }
    
    // 解析比较值
    Value compareVal = parseValue(type, parts[3]);
    if (!lastError.empty()) {
        return false;
    }
    
    // 检查类型是否匹配
    if (it->second.value.getType() != type) {
        lastError = "Type mismatch in condition";
        return false;
    }
    
    // 执行比较
    if (type == DataType::INT) {
        return it->second.value.getInt() == compareVal.getInt();
    } else if (type == DataType::STR) {
        return it->second.value.getStr() == compareVal.getStr();
    }
    
    return false;
}

// 查找匹配的else或end if
int MaskVM::findMatchingStatement(int startLine, int id, const std::string& target) {
    int nestingLevel = 0;
    
    for (int i = startLine; i < codeLines.size(); ++i) {
        std::string line = codeLines[i];
        
        // 跳过注释
        size_t commentPos = line.find('?');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        
        // 去除空格
        line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](int ch) {
            return !std::isspace(ch);
        }));
        
        if (line.empty()) continue;
        
        // 检查if语句，增加嵌套级别
        if (line.substr(0, 2) == "if") {
            nestingLevel++;
        }
        // 检查目标语句，减少嵌套级别
        else if (line.substr(0, target.size()) == target) {
            // 提取ID
            size_t spacePos = line.find(' ');
            if (spacePos == std::string::npos) continue;
            
            std::string idStr = line.substr(spacePos + 1);
            int currentId;
            try {
                currentId = std::stoi(idStr);
            } catch (...) {
                continue;
            }
            
            if (currentId == id) {
                nestingLevel--;
                if (nestingLevel == 0) {
                    return i;
                }
            }
        }
    }
    
    lastError = "Matching " + target + " not found for id: " + std::to_string(id);
    return -1;
}

// 加载代码
bool MaskVM::loadCode(const std::string& code) {
    codeLines.clear();
    variables.clear();
    lastError.clear();
    
    std::istringstream iss(code);
    std::string line;
    
    while (std::getline(iss, line)) {
        codeLines.push_back(line);
    }
    
    return true;
}

// 执行代码
bool MaskVM::execute() {
    lastError.clear();
    int lineNumber = 0;
    
    while (lineNumber < codeLines.size()) {
        std::string originalLine = codeLines[lineNumber];
        std::string line = originalLine;
        
        // 处理注释
        size_t commentPos = line.find('?');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        
        // 去除空格
        line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](int ch) {
            return !std::isspace(ch);
        }));
        
        if (line.empty()) {
            lineNumber++;
            continue;
        }
        
        // 分割指令
        std::vector<std::string> tokens = split(line, ' ');
        if (tokens.empty()) {
            lineNumber++;
            continue;
        }
        
        std::string command = tokens[0];
        
        // 处理变量定义 (def)
        if (command == "def") {
            if (tokens.size() < 4) {
                lastError = "Invalid def syntax at line " + std::to_string(lineNumber + 1);
                return false;
            }
            
            DataType type = parseDataType(tokens[1]);
            if (type == DataType::UNKNOWN) {
                lastError = "Unknown type in def at line " + std::to_string(lineNumber + 1);
                return false;
            }
            
            std::string varName = tokens[2];
            if (variables.find(varName) != variables.end()) {
                lastError = "Variable already exists: " + varName + " at line " + std::to_string(lineNumber + 1);
                return false;
            }
            
            Value value = parseValue(parseDataType(tokens[3]), tokens[4]);
            if (!lastError.empty()) {
                lastError += " at line " + std::to_string(lineNumber + 1);
                return false;
            }
            
            variables[varName] = {false, value}; // 不是常量
        }
        // 处理常量定义 (let)
        else if (command == "let") {
            if (tokens.size() < 4) {
                lastError = "Invalid let syntax at line " + std::to_string(lineNumber + 1);
                return false;
            }
            
            DataType type = parseDataType(tokens[1]);
            if (type == DataType::UNKNOWN) {
                lastError = "Unknown type in let at line " + std::to_string(lineNumber + 1);
                return false;
            }
            
            std::string varName = tokens[2];
            if (variables.find(varName) != variables.end()) {
                lastError = "Variable already exists: " + varName + " at line " + std::to_string(lineNumber + 1);
                return false;
            }
            
            Value value = parseValue(parseDataType(tokens[3]), tokens[4]);
            if (!lastError.empty()) {
                lastError += " at line " + std::to_string(lineNumber + 1);
                return false;
            }
            
            variables[varName] = {true, value}; // 是常量
        }
        // 处理赋值 (set)
        else if (command == "set") {
            if (tokens.size() < 4) {
                lastError = "Invalid set syntax at line " + std::to_string(lineNumber + 1);
                return false;
            }
            
            DataType type = parseDataType(tokens[1]);
            if (type == DataType::UNKNOWN) {
                lastError = "Unknown type in set at line " + std::to_string(lineNumber + 1);
                return false;
            }
            
            std::string varName = tokens[2];
            auto it = variables.find(varName);
            if (it == variables.end()) {
                lastError = "Variable not found: " + varName + " at line " + std::to_string(lineNumber + 1);
                return false;
            }
            
            // 检查是否为常量
            if (it->second.isConstant) {
                lastError = "Cannot set a constant: " + varName + " at line " + std::to_string(lineNumber + 1);
                return false;
            }
            
            Value value = parseValue(parseDataType(tokens[3]), tokens[4]);
            if (!lastError.empty()) {
                lastError += " at line " + std::to_string(lineNumber + 1);
                return false;
            }
            
            it->second.value = value;
        }
        // 处理打印 (print)
        else if (command == "print") {
            if (tokens.size() < 2) {
                lastError = "Invalid print syntax at line " + std::to_string(lineNumber + 1);
                return false;
            }
            
            if (tokens[1] == "str") {
                // 打印字符串常量
                std::string str = originalLine;
                size_t strPos = str.find("str \"");
                if (strPos == std::string::npos) {
                    lastError = "Invalid string in print at line " + std::to_string(lineNumber + 1);
                    return false;
                }
                
                strPos += 5; // 跳过 "str \""
                size_t endPos = str.find("\"", strPos);
                if (endPos == std::string::npos) {
                    lastError = "Unclosed string in print at line " + std::to_string(lineNumber + 1);
                    return false;
                }
                
                output << str.substr(strPos, endPos - strPos) << std::endl;
            } else {
                // 打印变量
                std::string varName = tokens[1];
                auto it = variables.find(varName);
                if (it == variables.end()) {
                    lastError = "Variable not found: " + varName + " at line " + std::to_string(lineNumber + 1);
                    return false;
                }
                
                output << it->second.value.toString() << std::endl;
            }
        }
        // 处理if语句
        else if (command == "if") {
            if (tokens.size() < 2) {
                lastError = "Invalid if syntax at line " + std::to_string(lineNumber + 1);
                return false;
            }
            
            // 提取if语句ID
            int ifId;
            try {
                ifId = std::stoi(tokens[1]);
            } catch (...) {
                lastError = "Invalid if ID at line " + std::to_string(lineNumber + 1);
                return false;
            }
            
            // 提取条件表达式
            std::string conditionLine = originalLine;
            size_t ifPos = conditionLine.find("if " + std::to_string(ifId) + " ");
            if (ifPos == std::string::npos) {
                lastError = "Invalid if syntax at line " + std::to_string(lineNumber + 1);
                return false;
            }
            
            size_t conditionStart = ifPos + 4 + std::to_string(ifId).size(); // "if x " 的长度
            size_t commentStart = conditionLine.find('?', conditionStart);
            std::string condition;
            
            if (commentStart != std::string::npos) {
                condition = conditionLine.substr(conditionStart, commentStart - conditionStart);
            } else {
                condition = conditionLine.substr(conditionStart);
            }
            
            // 评估条件
            bool conditionResult = evaluateCondition(condition);
            if (!lastError.empty()) {
                lastError += " at line " + std::to_string(lineNumber + 1);
                return false;
            }
            
            // 查找else和end if
            int elseLine = findMatchingStatement(lineNumber + 1, ifId, "else");
            int endIfLine = findMatchingStatement(lineNumber + 1, ifId, "end if");
            
            if (elseLine == -1 || endIfLine == -1) {
                return false;
            }
            
            // 根据条件结果跳转到相应的代码块
            if (conditionResult) {
                // 条件为真，执行到else之前
                lineNumber++;
            } else {
                // 条件为假，跳转到else之后
                lineNumber = elseLine + 1;
            }
            
            continue; // 跳过正常的lineNumber++
        }
        // 处理else语句
        else if (command == "else") {
            if (tokens.size() < 2) {
                lastError = "Invalid else syntax at line " + std::to_string(lineNumber + 1);
                return false;
            }
            
            // 提取else语句ID
            int elseId;
            try {
                elseId = std::stoi(tokens[1]);
            } catch (...) {
                lastError = "Invalid else ID at line " + std::to_string(lineNumber + 1);
                return false;
            }
            
            // 跳转到对应的end if
            int endIfLine = findMatchingStatement(lineNumber + 1, elseId, "end if");
            if (endIfLine == -1) {
                return false;
            }
            
            lineNumber = endIfLine + 1;
            continue; // 跳过正常的lineNumber++
        }
        // 处理end if语句
        else if (command == "end" && tokens.size() > 1 && tokens[1] == "if") {
            // 什么都不做，继续执行下一行
        }
        // 未知指令
        else {
            lastError = "Unknown command: " + command + " at line " + std::to_string(lineNumber + 1);
            return false;
        }
        
        lineNumber++;
    }
    
    return true;
}
