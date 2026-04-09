#pragma once
#include "visitor.h"
#include "symbol_table.h"
#include "scope_tree.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>

#include <string>
#include <unordered_map>
#include <memory>

class LLVMVisitor : public Visitor {
public:
    explicit LLVMVisitor(const SymbolTable& symTable,
                         const std::string& moduleName = "module");

    void visit(Program*         node) override;
    void visit(DeclareStmt*     node) override;
    void visit(AssignStmt*      node) override;
    void visit(PrintStmt*       node) override;
    void visit(IfStmt*          node) override;
    void visit(NumberExpr*      node) override;
    void visit(VariableExpr*    node) override;
    void visit(BinaryExpr*      node) override;
    void visit(EqualExpr*       node) override;

    void visit(ClassDecl*       node) override;
    void visit(FieldDecl*       node) override;
    void visit(MethodDecl*      node) override;
    void visit(ReturnStmt*      node) override;
    void visit(VarDeclStmt*     node) override;
    void visit(NewObjectExpr*   node) override;
    void visit(NewArrayExpr*    node) override;
    void visit(MethodCallExpr*  node) override;
    void visit(FieldAccessExpr* node) override;
    void visit(ThisExpr*        node) override;
    void visit(ArrayAccessExpr* node) override;
    void visit(ArrayLengthExpr* node) override;

    void printIR() const;
    void saveIR(const std::string& filename) const;
    llvm::Module& getModule() { return *module_; }

private:
    mutable llvm::LLVMContext          context_;
    std::unique_ptr<llvm::Module>      module_;
    llvm::IRBuilder<>                  builder_;
    const SymbolTable&                 symTable_;

    llvm::Value* lastValue_ = nullptr;
    std::string  currentClass_;
    std::string  currentMethod_;
    llvm::Value* thisPtr_ = nullptr;

    std::unordered_map<std::string, llvm::Value*>       localVars_;
    std::unordered_map<std::string, llvm::StructType*>  classTypes_;
    std::unordered_map<std::string, llvm::Function*>    methodFunctions_;

    llvm::Function* printfFunc_ = nullptr;
    llvm::Function* mallocFunc_ = nullptr;

    void declareRuntimeFunctions();
    void declareClassTypes();
    void declareMethodPrototypes();
    void emitMainFunction(Program* node);

    llvm::Type*  llvmTypeFor(const TypeInfo& ti) const;
    llvm::Type*  llvmTypeFor(const std::string& typeName, bool isArray) const;
    llvm::Value* loadVar(const std::string& name);
    llvm::Value* ptrForVar(const std::string& name);
    llvm::Value* evalExpr(class Expression* expr);
    int          fieldIndex(const std::string& className, const std::string& fieldName) const;

    static std::string mangleMethod(const std::string& className,
                                    const std::string& methodName);
};
