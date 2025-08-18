#include <codegen.hpp>

CodeGenerator::CodeGenerator() 
    : module(std::make_unique<llvm::Module>("main", context)),
      builder(std::make_unique<llvm::IRBuilder<>>(context)) {
    declareRuntimeFunctions(); // 声明几个函数（隐式转换函数总行了吧。。。）
}


