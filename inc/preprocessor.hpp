#pragma once

#include <string>
#include <fstream>
#include <filesystem>
#include <vector>
#include <regex>
#include <stdexcept>
#include <sstream>
#include <unordered_set>

class Preprocessor {
public:
    Preprocessor(std::string source, const std::string& currentFilePath)
        : sourceCode(std::move(source)), 
          currentDir(std::filesystem::path(currentFilePath).parent_path()) {}

    std::string process();

    const std::vector<std::string>& getDynamicLibraries() const;

private:
    std::string sourceCode;
    std::filesystem::path currentDir;
    std::vector<std::string> dynamicLibraries;
    std::unordered_set<std::string> processedFiles;

    void processImports();

    std::filesystem::path resolveImportFile(const std::string& importName);

    void processDynamicLibs(const std::string& content, const std::filesystem::path& libDir);

    std::string getPlatformLibPath(const std::string& libName, const std::filesystem::path& baseDir);

    std::string readFile(const std::string& path);
};
