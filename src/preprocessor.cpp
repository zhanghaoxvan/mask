#include "preprocessor.hpp"


std::string Preprocessor::process() {
    processImports();
    return sourceCode;
}

const std::vector<std::string>& Preprocessor::getDynamicLibraries() const {
    return dynamicLibraries;
}

void Preprocessor::processImports() {
            const std::regex importRegex("^\\s*import\\s+\"([^\"]+)\"\\s*(->\\s*(\\w+))?\\s*;.*", std::regex::multiline);
    std::smatch match;

    while (std::regex_search(sourceCode, match, importRegex)) {
        std::string importName = match[1].str();
        std::string alias = match[3].str();

        std::filesystem::path importPath = resolveImportFile(importName);
        std::string importPathStr = importPath.string();

        if (processedFiles.count(importPathStr)) {
            sourceCode = match.prefix().str() + match.suffix().str();
            continue;
        }

        processedFiles.insert(importPathStr);
        std::string importContent = readFile(importPathStr);

        processDynamicLibs(importContent, importPath.parent_path());

        Preprocessor subProcessor(importContent, importPathStr);
        std::string processedImportContent = subProcessor.process();

        sourceCode = match.prefix().str() + processedImportContent + match.suffix().str();

        const auto& subLibs = subProcessor.getDynamicLibraries();
        dynamicLibraries.insert(
            dynamicLibraries.end(), 
            subLibs.begin(), 
            subLibs.end()
        );
    }
}

std::filesystem::path Preprocessor::resolveImportFile(const std::string& importName) {
    std::vector<std::filesystem::path> searchPaths = {
        currentDir / "lib" / (importName + ".ma"),
        currentDir / "lib" / importName / (importName + ".ma"),
        currentDir / "lib" / "std" / (importName + ".ma") // Mask Standard Libs路径
    };

    for (const auto& path : searchPaths) {
        if (std::filesystem::exists(path) && std::filesystem::is_regular_file(path)) {
            return path;
        }
    }

    throw std::runtime_error("Import not found: " + importName);
}

void Preprocessor::processDynamicLibs(const std::string& content, const std::filesystem::path& libDir) {
    const std::regex dynLibRegex("^\\s*dynamicLib\\s*\\(\"([^\"]+)\"\\)\\s*;.*", std::regex::multiline);
    std::smatch match;
    std::string tempContent = content;

    while (std::regex_search(tempContent, match, dynLibRegex)) {
        std::string libName = match[1].str();
        std::string libPath = getPlatformLibPath(libName, libDir);

        if (!std::filesystem::exists(libPath)) {
            throw std::runtime_error("Dynamic library not found: " + libPath);
        }

        if (std::find(dynamicLibraries.begin(), dynamicLibraries.end(), libPath) == dynamicLibraries.end()) {
            dynamicLibraries.push_back(libPath);
        }

        tempContent = match.suffix().str();
    }
}

std::string Preprocessor::getPlatformLibPath(const std::string& libName, const std::filesystem::path& baseDir) {
#ifdef _WIN32
    return (baseDir / ("lib" + libName + ".dll")).string();
#elif __linux__
    return (baseDir / ("lib" + libName + ".so")).string();
#elif __APPLE__
    return (baseDir / ("lib" + libName + ".dylib")).string();
#else
    throw std::runtime_error("Unsupported platform");
#endif
}

std::string Preprocessor::readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
