#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <sstream>
#include <algorithm>

// 变量类型枚举
enum class DataType {
    INT,
    STR,
    UNKNOWN
};

// 变量值的包装类
class Value {
private:
    DataType type;
    int intVal;
    std::string strVal;

public:
    Value() : type(DataType::UNKNOWN), intVal(0) {}
    Value(int val) : type(DataType::INT), intVal(val) {}
    Value(const std::string& val) : type(DataType::STR), strVal(val) {}

    DataType getType() const { return type; }
    int getInt() const { return intVal; }
    std::string getStr() const { return strVal; }

    std::string toString() const {
        if (type == DataType::INT) {
            return std::to_string(intVal);
        } else if (type == DataType::STR) {
            return strVal;
        }
        return "unknown";
    }
};

// 变量信息结构体
struct Variable {
    bool isConstant;
    Value value;
};

// Mask VM 解释器类
class MaskVM {
private:
    // 存储变量和常量
    std::unordered_map<std::string, Variable> variables;
    // 存储代码行
    std::vector<std::string> codeLines;
    // 输出流
    std::ostream& output;
    // 错误信息
    std::string lastError;

    // 解析数据类型
    DataType parseDataType(const std::string& typeStr);
    
    // 解析值
    Value parseValue(DataType type, const std::string& valueStr);
    
    // 解析条件表达式
    bool evaluateCondition(const std::string& condition);
    
    // 查找匹配的else或end if
    int findMatchingStatement(int startLine, int id, const std::string& target);

public:
    MaskVM(std::ostream& output = std::cout) : output(output) {}
    
    // 加载代码
    bool loadCode(const std::string& code);
    
    // 执行代码
    bool execute();
    
    // 获取最后一条错误信息
    std::string getLastError() const { return lastError; }
};
