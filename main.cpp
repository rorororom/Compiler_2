#include "lexer.h"
#include "parser.h"
#include "print_visitor.h"
#include "scope_visitor.h"
#include "llvm_visitor.h"

#include <llvm/Support/CommandLine.h>
#include <llvm/Support/TargetSelect.h>

#include <fstream>
#include <sstream>

static llvm::cl::opt<std::string> InputFile(
    llvm::cl::Positional,
    llvm::cl::desc("<source file>"),
    llvm::cl::Required);

static llvm::cl::opt<std::string> OutputIR(
    "o",
    llvm::cl::desc("Output .ll file (default: output.ll)"),
    llvm::cl::value_desc("filename"),
    llvm::cl::init("output.ll"));

static llvm::cl::opt<std::string> EmitObj(
    "emit-obj",
    llvm::cl::desc("Emit native object file (.o) to the given path"),
    llvm::cl::value_desc("filename"),
    llvm::cl::init(""));

static llvm::cl::opt<std::string> CompileOut(
    "compile",
    llvm::cl::desc("Compile to native executable (implies --emit-obj)"),
    llvm::cl::value_desc("filename"),
    llvm::cl::init(""));

static llvm::cl::opt<int> OptLevel(
    "opt-level",
    llvm::cl::desc("Optimisation level: 0 (none), 1, 2, 3 (default: 0)"),
    llvm::cl::value_desc("N"),
    llvm::cl::init(0));

static llvm::cl::opt<bool> PrintAST(
    "print-ast",
    llvm::cl::desc("Write AST to ast_output.txt"),
    llvm::cl::init(false));

static llvm::cl::opt<bool> PrintIR(
    "print-ir",
    llvm::cl::desc("Print LLVM IR to stdout"),
    llvm::cl::init(false));

static llvm::cl::opt<bool> DumpScope(
    "dump-scope",
    llvm::cl::desc("Dump scope tree to scope_tree_output.txt"),
    llvm::cl::init(false));

int main(int argc, char* argv[]) {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmParser();
    llvm::InitializeNativeTargetAsmPrinter();

    llvm::cl::ParseCommandLineOptions(argc, argv, "Mini-language compiler\n");

    std::ifstream file(InputFile);
    if (!file.is_open()) {
        llvm::errs() << "Error: cannot open file '" << InputFile << "'\n";
        return 1;
    }
    std::ostringstream ss;
    ss << file.rdbuf();

    try {
        Lexer lexer(ss.str());
        auto tokens = lexer.tokenize();

        Parser parser(tokens);
        auto program = parser.parseProgram();

        if (PrintAST) {
            PrintVisitor printVisitor("ast_output.txt");
            program->accept(&printVisitor);
            llvm::outs() << "AST written to ast_output.txt\n";
        }

        ScopeVisitor scopeVisitor;
        scopeVisitor.collectClasses(program.get());
        program->accept(&scopeVisitor);

        if (DumpScope) {
            std::ofstream scopeOut("scope_tree_output.txt");
            if (scopeOut.is_open())
                scopeVisitor.getScopeTree().dump(scopeOut);
        }

        llvm::outs() << "Semantic analysis passed.\n";

        LLVMVisitor llvmVisitor(scopeVisitor.getSymbolTable(), InputFile);
        program->accept(&llvmVisitor);

        llvmVisitor.saveIR(OutputIR);
        llvm::outs() << "LLVM IR written to " << OutputIR << "\n";

        if (PrintIR) {
            llvm::outs() << "\n=== LLVM IR ===\n";
            llvmVisitor.printIR();
        }

        if (!EmitObj.empty()) {
            llvmVisitor.compileToObject(EmitObj, OptLevel);
            llvm::outs() << "Native object file written to " << EmitObj << "\n";
        }

        if (!CompileOut.empty()) {
            llvmVisitor.compileToExecutable(CompileOut, OptLevel);
            llvm::outs() << "Executable written to " << CompileOut << "\n";
        }

        if (EmitObj.empty() && CompileOut.empty()) {
            llvm::outs() << "Tip: use --compile <outfile> to produce a native executable "
                            "(opt-level 0-3 via --opt-level <N>)\n";
        }

    } catch (const std::exception& e) {
        llvm::errs() << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
