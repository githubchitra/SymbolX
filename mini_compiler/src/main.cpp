#include "compiler.h"
#include "grammar_interface.h"
#include "ast_visualizer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// Test cases
const std::vector<std::pair<std::string, std::string>> TEST_CASES = {
    {
        "Simple Variable Declaration",
        R"(
int main() {
    int x = 5;
    int y = 10;
    return x + y;
}
)"
    },
    {
        "Function with Parameters",
        R"(
int add(int a, int b) {
    return a + b;
}

int main() {
    int result = add(5, 3);
    return result;
}
)"
    },
    {
        "Control Flow - If Statement",
        R"(
int max(int a, int b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

int main() {
    int x = max(10, 5);
    return x;
}
)"
    },
    {
        "Loop - While Statement",
        R"(
int factorial(int n) {
    int result = 1;
    while (n > 1) {
        result = result * n;
        n = n - 1;
    }
    return result;
}

int main() {
    return factorial(5);
}
)"
    },
    {
        "Loop - For Statement",
        R"(
int sum(int n) {
    int result = 0;
    for (int i = 1; i <= n; i = i + 1) {
        result = result + i;
    }
    return result;
}

int main() {
    return sum(10);
}
)"
    },
    {
        "Multiple Data Types",
        R"(
float calculate(int a, float b) {
    return a + b;
}

int main() {
    int x = 5;
    float y = 3.14;
    float result = calculate(x, y);
    return (int)result;
}
)"
    },
    {
        "Nested Scopes",
        R"(
int test() {
    int x = 10;
    {
        int y = 20;
        x = x + y;
    }
    return x;
}

int main() {
    return test();
}
)"
    }
};

// Error test cases
const std::vector<std::pair<std::string, std::string>> ERROR_TEST_CASES = {
    {
        "Undeclared Variable",
        R"(
int main() {
    int x = undeclared_var + 5;
    return x;
}
)"
    },
    {
        "Type Mismatch",
        R"(
int main() {
    int x = "hello";
    return x;
}
)"
    },
    {
        "Redeclaration",
        R"(
int main() {
    int x = 5;
    int x = 10;
    return x;
}
)"
    },
    {
        "Invalid Assignment",
        R"(
int main() {
    int x = 5;
    x + 1 = 10;
    return x;
}
)"
    },
    {
        "Undefined Function",
        R"(
int main() {
    int result = undefined_function(5);
    return result;
}
)"
    }
};

void printHeader(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << title << "\n";
    std::cout << std::string(60, '=') << "\n";
}

void printTestResult(const std::string& testName, bool success, int errors, int warnings) {
    std::cout << "\n" << std::string(40, '-') << "\n";
    std::cout << "Test: " << testName << "\n";
    std::cout << "Result: " << (success ? "PASS" : "FAIL") << "\n";
    std::cout << "Errors: " << errors << "\n";
    std::cout << "Warnings: " << warnings << "\n";
    std::cout << std::string(40, '-') << "\n";
}

void runTest(const std::string& testName, const std::string& sourceCode, bool expectSuccess = true) {
    std::cout << "\nCompiling: " << testName << "\n";
    std::cout << "Source code:\n" << sourceCode << "\n";
    
    Compiler compiler(sourceCode);
    auto result = compiler.compile();
    
    bool success = (result == CompilationResult::SUCCESS);
    
    if (expectSuccess && !success) {
        std::cout << "UNEXPECTED FAILURE!\n";
        compiler.printErrors();
    } else if (!expectSuccess && success) {
        std::cout << "UNEXPECTED SUCCESS!\n";
    }
    
    printTestResult(testName, expectSuccess ? success : !success, 
                   compiler.getErrorCount(), compiler.getWarningCount());
    
    if (!expectSuccess && !success) {
        std::cout << "Expected errors detected:\n";
        compiler.printErrors();
    }
}

void runAllTests() {
    printHeader("Running All Test Cases");
    
    int passedTests = 0;
    int totalTests = static_cast<int>(TEST_CASES.size());
    
    for (const auto& testCase : TEST_CASES) {
        runTest(testCase.first, testCase.second, true);
        passedTests++;
    }
    
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "Valid Test Cases Summary\n";
    std::cout << std::string(60, '=') << "\n";
    std::cout << "Passed: " << passedTests << "/" << totalTests << "\n";
    std::cout << "Success Rate: " << (passedTests * 100 / totalTests) << "%\n";
}

void runErrorTests() {
    printHeader("Running Error Detection Tests");
    
    int detectedErrors = 0;
    int totalErrorTests = static_cast<int>(ERROR_TEST_CASES.size());
    
    for (const auto& testCase : ERROR_TEST_CASES) {
        runTest(testCase.first, testCase.second, false);
        detectedErrors++;
    }
    
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "Error Detection Summary\n";
    std::cout << std::string(60, '=') << "\n";
    std::cout << "Errors Detected: " << detectedErrors << "/" << totalErrorTests << "\n";
    std::cout << "Detection Rate: " << (detectedErrors * 100 / totalErrorTests) << "%\n";
}

void demonstrateFeatures() {
    printHeader("Mini Compiler Front-End Demonstration");
    
    std::string demoCode = R"(
// This is a sample program to demonstrate the compiler
int fibonacci(int n) {
    if (n <= 1) {
        return n;
    } else {
        return fibonacci(n - 1) + fibonacci(n - 2);
    }
}

int main() {
    int result = fibonacci(10);
    return result;
}
)";
    
    std::cout << "Sample Program:\n" << demoCode << "\n";
    
    Compiler compiler(demoCode);
    auto result = compiler.compile();
    
    if (result == CompilationResult::SUCCESS) {
        std::cout << "\nCompilation successful!\n\n";
        
        std::cout << "Generated Tokens:\n";
        compiler.printTokens();
        
        std::cout << "\nAbstract Syntax Tree:\n";
        compiler.printAST();
        
        std::cout << "\nSymbol Table:\n";
        compiler.printSymbolTable();
        
        std::cout << "\nCompilation Report:\n";
        compiler.printCompilationReport();
    } else {
        std::cout << "\nCompilation failed!\n";
        compiler.printErrors();
    }
}

void interactiveMode() {
    printHeader("Interactive Mode");
    std::cout << "Enter C code (type 'exit' to quit, 'help' for commands):\n";
    
    std::string line;
    std::string sourceCode;
    int lineNumber = 1;
    
    while (true) {
        std::cout << lineNumber << "> ";
        std::getline(std::cin, line);
        
        if (line == "exit") {
            break;
        } else if (line == "help") {
            std::cout << "Commands:\n";
            std::cout << "  exit    - Exit interactive mode\n";
            std::cout << "  clear   - Clear current input\n";
            std::cout << "  compile - Compile current input\n";
            std::cout << "  help    - Show this help\n";
            continue;
        } else if (line == "clear") {
            sourceCode.clear();
            lineNumber = 1;
            std::cout << "Input cleared.\n";
            continue;
        } else if (line == "compile") {
            if (sourceCode.empty()) {
                std::cout << "No input to compile.\n";
                continue;
            }
            
            Compiler compiler(sourceCode);
            auto result = compiler.compile();
            
            if (result == CompilationResult::SUCCESS) {
                std::cout << "\nCompilation successful!\n";
                compiler.printCompilationReport();
            } else {
                std::cout << "\nCompilation failed!\n";
                compiler.printErrors();
            }
            continue;
        }
        
        sourceCode += line + "\n";
        lineNumber++;
    }
}

void compileFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Error: Could not open file '" << filename << "'\n";
        return;
    }
    
    std::string sourceCode((std::istreambuf_iterator<char>(file)),
                          std::istreambuf_iterator<char>());
    file.close();
    
    std::cout << "Compiling file: " << filename << "\n";
    std::cout << "Source code:\n" << sourceCode << "\n";
    
    Compiler compiler(sourceCode);
    auto result = compiler.compile();
    
    if (result == CompilationResult::SUCCESS) {
        std::cout << "\nFile compiled successfully!\n";
        compiler.printDetailedReport();
    } else {
        std::cout << "\nFile compilation failed!\n";
        compiler.printErrors();
    }
}

void cfgGrammarMode() {
    printHeader("CFG Grammar Parsing Mode");
    
    GrammarInterface grammarInterface;
    std::cout << grammarInterface.getHelp() << "\n";
    
    while (true) {
        std::cout << "\nCFG> ";
        std::string command;
        std::getline(std::cin, command);
        
        // Parse command
        std::istringstream iss(command);
        std::string cmd;
        iss >> cmd;
        
        if (cmd == "exit" || cmd == "quit") {
            break;
        } else if (cmd == "help") {
            std::cout << grammarInterface.getHelp() << "\n";
        } else if (cmd == "load") {
            std::string arg;
            iss >> arg;
            if (arg == "interactive") {
                grammarInterface.loadGrammarFromInteractive();
            } else {
                if (grammarInterface.loadGrammarFromFile(arg)) {
                    std::cout << "Grammar loaded successfully from " << arg << "\n";
                } else {
                    std::cout << "Failed to load grammar from " << arg << "\n";
                    grammarInterface.printErrors();
                }
            }
        } else if (cmd == "load-interactive") {
            grammarInterface.loadGrammarFromInteractive();
        } else if (cmd == "load-expression") {
            if (grammarInterface.loadExpressionGrammar()) {
                std::cout << "Expression grammar loaded successfully.\n";
            } else {
                std::cout << "Failed to load expression grammar.\n";
            }
        } else if (cmd == "load-arithmetic") {
            if (grammarInterface.loadSimpleArithmeticGrammar()) {
                std::cout << "Arithmetic grammar loaded successfully.\n";
            } else {
                std::cout << "Failed to load arithmetic grammar.\n";
            }
        } else if (cmd == "load-statement") {
            if (grammarInterface.loadStatementGrammar()) {
                std::cout << "Statement grammar loaded successfully.\n";
            } else {
                std::cout << "Failed to load statement grammar.\n";
            }
        } else if (cmd == "analyze") {
            if (grammarInterface.analyzeGrammar()) {
                std::cout << "Grammar analysis completed.\n";
                grammarInterface.printFirstFollowSets();
            } else {
                std::cout << "Grammar analysis failed.\n";
            }
        } else if (cmd == "build-table") {
            if (grammarInterface.buildParsingTable()) {
                std::cout << "Parsing table built successfully.\n";
                grammarInterface.printParsingTable();
            } else {
                std::cout << "Failed to build parsing table.\n";
                grammarInterface.printErrors();
            }
        } else if (cmd == "parse") {
            std::string input;
            std::getline(iss >> std::ws, input);
            if (!input.empty()) {
                if (grammarInterface.parseInput(input)) {
                    std::cout << "Parsing successful!\n";
                    grammarInterface.printParseTree();
                } else {
                    std::cout << "Parsing failed!\n";
                    grammarInterface.printErrors();
                }
            } else {
                std::cout << "Usage: parse <input>\n";
            }
        } else if (cmd == "parse-interactive") {
            grammarInterface.parseInteractive();
        } else if (cmd == "print-grammar") {
            grammarInterface.printGrammar();
        } else if (cmd == "print-first-follow") {
            grammarInterface.printFirstFollowSets();
        } else if (cmd == "print-table") {
            grammarInterface.printParsingTable();
        } else if (cmd == "print-tree") {
            grammarInterface.printParseTree();
        } else if (cmd == "print-steps") {
            grammarInterface.printParsingSteps();
        } else if (cmd == "print-errors") {
            grammarInterface.printErrors();
        } else if (cmd == "print-warnings") {
            grammarInterface.printWarnings();
        } else if (cmd == "print-report") {
            grammarInterface.printFullReport();
        } else if (cmd == "export-dot") {
            std::string filename;
            iss >> filename;
            if (!filename.empty()) {
                if (grammarInterface.exportParseTreeDOT(filename)) {
                    std::cout << "Parse tree exported to " << filename << "\n";
                } else {
                    std::cout << "Failed to export parse tree.\n";
                }
            } else {
                std::cout << "Usage: export-dot <filename>\n";
            }
        } else if (cmd == "export-json") {
            std::string filename;
            iss >> filename;
            if (!filename.empty()) {
                if (grammarInterface.exportParseTreeJSON(filename)) {
                    std::cout << "Parse tree exported to " << filename << "\n";
                } else {
                    std::cout << "Failed to export parse tree.\n";
                }
            } else {
                std::cout << "Usage: export-json <filename>\n";
            }
        } else if (cmd == "export-csv") {
            std::string filename;
            iss >> filename;
            if (!filename.empty()) {
                if (grammarInterface.exportParsingTableCSV(filename)) {
                    std::cout << "Parsing table exported to " << filename << "\n";
                } else {
                    std::cout << "Failed to export parsing table.\n";
                }
            } else {
                std::cout << "Usage: export-csv <filename>\n";
            }
        } else if (cmd == "status") {
            std::cout << grammarInterface.getStatus() << "\n";
        } else if (cmd == "reset") {
            grammarInterface.reset();
            std::cout << "Components reset.\n";
        } else if (!cmd.empty()) {
            std::cout << "Unknown command: " << cmd << "\n";
            std::cout << "Type 'help' for available commands.\n";
        }
    }
}

void printUsage(const std::string& programName) {
    std::cout << "Mini Compiler Front-End Usage:\n";
    std::cout << "  " << programName << "                    - Run interactive mode\n";
    std::cout << "  " << programName << " test                - Run all test cases\n";
    std::cout << "  " << programName << " error               - Run error detection tests\n";
    std::cout << "  " << programName << " demo                - Run demonstration\n";
    std::cout << "  " << programName << " cfg                 - Enter CFG grammar parsing mode\n";
    std::cout << "  " << programName << " <filename>          - Compile file\n";
    std::cout << "  " << programName << " help                - Show this help\n";
}

int main(int argc, char* argv[]) {
    std::string programName = argv[0];
    
    if (argc == 1) {
        // Interactive mode
        interactiveMode();
    } else if (argc == 2) {
        std::string arg = argv[1];
        
        if (arg == "test") {
            runAllTests();
        } else if (arg == "error") {
            runErrorTests();
        } else if (arg == "demo") {
            demonstrateFeatures();
        } else if (arg == "cfg") {
            cfgGrammarMode();
        } else if (arg == "help") {
            printUsage(programName);
        } else {
            // Compile from file
            compileFromFile(arg);
        }
    } else {
        printUsage(programName);
        return 1;
    }
    
    return 0;
}
