#include "llvm_visitor.h"
#include "statements.h"
#include "expressions.h"
#include "oop_nodes.h"
#include "program.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>

#include <stdexcept>

LLVMVisitor::LLVMVisitor(const SymbolTable& symTable,
                         const std::string& moduleName)
    : module_(std::make_unique<llvm::Module>(moduleName, context_))
    , builder_(context_)
    , symTable_(symTable)
{
    declareRuntimeFunctions();
}

void LLVMVisitor::declareRuntimeFunctions() {
    llvm::Type* i8ptr = llvm::PointerType::get(llvm::Type::getInt8Ty(context_), 0);

    llvm::FunctionType* printfTy = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(context_), {i8ptr}, true);
    printfFunc_ = llvm::Function::Create(
        printfTy, llvm::Function::ExternalLinkage, "printf", module_.get());

    llvm::FunctionType* mallocTy = llvm::FunctionType::get(
        i8ptr, {llvm::Type::getInt64Ty(context_)}, false);
    mallocFunc_ = llvm::Function::Create(
        mallocTy, llvm::Function::ExternalLinkage, "malloc", module_.get());
}

llvm::Type* LLVMVisitor::llvmTypeFor(const TypeInfo& ti) const {
    bool isArr = (ti.kind == VarKind::ARRAY_INT || ti.kind == VarKind::ARRAY_CLASS);
    std::string name = (ti.kind == VarKind::INT || ti.kind == VarKind::ARRAY_INT)
                           ? "int" : ti.className;
    return llvmTypeFor(name, isArr);
}

llvm::Type* LLVMVisitor::llvmTypeFor(const std::string& typeName, bool isArray) const {
    llvm::Type* base = nullptr;
    if (typeName == "int") {
        base = llvm::Type::getInt32Ty(context_);
    } else if (typeName == "void") {
        base = llvm::Type::getVoidTy(context_);
    } else {
        auto it = classTypes_.find(typeName);
        base = (it != classTypes_.end())
            ? llvm::PointerType::get(it->second, 0)
            : llvm::PointerType::get(llvm::Type::getInt8Ty(context_), 0);
    }
    if (isArray)
        return llvm::PointerType::get(llvm::Type::getInt32Ty(context_), 0);
    return base;
}

std::string LLVMVisitor::mangleMethod(const std::string& className,
                                       const std::string& methodName) {
    return className + "__" + methodName;
}

int LLVMVisitor::fieldIndex(const std::string& className,
                             const std::string& fieldName) const {
    const ClassSymbol* cs = symTable_.findClass(className);
    if (!cs) return -1;
    const VariableSymbol* f = cs->findField(fieldName);
    return f ? f->index : -1;
}

void LLVMVisitor::declareClassTypes() {
    for (auto& [name, cls] : symTable_.allClasses())
        classTypes_[name] = llvm::StructType::create(context_, name);

    for (auto& [name, cls] : symTable_.allClasses()) {
        std::vector<llvm::Type*> fieldTypes;
        for (auto& f : cls.fields)
            fieldTypes.push_back(llvmTypeFor(f.type));
        classTypes_[name]->setBody(fieldTypes);
    }
}

void LLVMVisitor::declareMethodPrototypes() {
    for (auto& [className, cls] : symTable_.allClasses()) {
        llvm::Type* selfPtrTy = llvm::PointerType::get(classTypes_[className], 0);
        for (auto& [mname, ms] : cls.methods) {
            if (ms.isConstructor) continue;
            std::vector<llvm::Type*> paramTypes = {selfPtrTy};
            for (auto& p : ms.params)
                paramTypes.push_back(llvmTypeFor(p.type));
            llvm::FunctionType* ft = llvm::FunctionType::get(
                llvmTypeFor(ms.returnType), paramTypes, false);
            std::string mangledName = mangleMethod(className, mname);
            llvm::Function* fn = llvm::Function::Create(
                ft, llvm::Function::ExternalLinkage, mangledName, module_.get());
            auto argIt = fn->arg_begin();
            argIt->setName("self"); ++argIt;
            for (auto& p : ms.params) { argIt->setName(p.name); ++argIt; }
            methodFunctions_[mangledName] = fn;
        }
    }
}

llvm::Value* LLVMVisitor::ptrForVar(const std::string& name) {
    auto lit = localVars_.find(name);
    if (lit != localVars_.end()) return lit->second;

    if (thisPtr_ && !currentClass_.empty()) {
        int idx = fieldIndex(currentClass_, name);
        if (idx >= 0)
            return builder_.CreateStructGEP(classTypes_[currentClass_], thisPtr_,
                                            static_cast<unsigned>(idx), name + "_ptr");
    }
    throw std::runtime_error("CodegenError: unknown variable '" + name + "'");
}

llvm::Value* LLVMVisitor::loadVar(const std::string& name) {
    return builder_.CreateLoad(llvm::Type::getInt32Ty(context_), ptrForVar(name), name);
}

llvm::Value* LLVMVisitor::evalExpr(Expression* expr) {
    expr->accept(this);
    return lastValue_;
}

void LLVMVisitor::visit(Program* node) {
    declareClassTypes();
    declareMethodPrototypes();
    for (auto& stmt : node->getStatements())
        if (auto* cls = dynamic_cast<ClassDecl*>(stmt.get()))
            cls->accept(this);
    emitMainFunction(node);
}

void LLVMVisitor::emitMainFunction(Program* node) {
    llvm::FunctionType* mainTy = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(context_), {}, false);
    llvm::Function* mainFn = llvm::Function::Create(
        mainTy, llvm::Function::ExternalLinkage, "main", module_.get());
    builder_.SetInsertPoint(
        llvm::BasicBlock::Create(context_, "entry", mainFn));

    localVars_.clear();
    thisPtr_ = nullptr;
    currentClass_.clear();
    currentMethod_.clear();

    for (auto& stmt : node->getStatements())
        if (!dynamic_cast<ClassDecl*>(stmt.get()))
            stmt->accept(this);

    builder_.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), 0));
    llvm::verifyFunction(*mainFn);
}

void LLVMVisitor::visit(DeclareStmt* node) {
    llvm::AllocaInst* alloca = builder_.CreateAlloca(
        llvm::Type::getInt32Ty(context_), nullptr, node->getName());
    builder_.CreateStore(
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), 0), alloca);
    localVars_[node->getName()] = alloca;
}

void LLVMVisitor::visit(AssignStmt* node) {
    builder_.CreateStore(evalExpr(node->getExpression()), ptrForVar(node->getName()));
}

void LLVMVisitor::visit(PrintStmt* node) {
    builder_.CreateCall(printfFunc_,
        {builder_.CreateGlobalStringPtr("%d\n", "fmt"), evalExpr(node->getExpression())});
}

void LLVMVisitor::visit(IfStmt* node) {
    llvm::Function* fn = builder_.GetInsertBlock()->getParent();
    llvm::Value* condBool = builder_.CreateICmpNE(
        evalExpr(node->getCondition()),
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), 0), "ifcond");

    llvm::BasicBlock* thenBB  = llvm::BasicBlock::Create(context_, "then",  fn);
    llvm::BasicBlock* elseBB  = llvm::BasicBlock::Create(context_, "else",  fn);
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(context_, "merge", fn);
    builder_.CreateCondBr(condBool, thenBB, elseBB);

    builder_.SetInsertPoint(thenBB);
    for (auto& s : node->getThenBranch()) s->accept(this);
    if (!builder_.GetInsertBlock()->getTerminator()) builder_.CreateBr(mergeBB);

    builder_.SetInsertPoint(elseBB);
    for (auto& s : node->getElseBranch()) s->accept(this);
    if (!builder_.GetInsertBlock()->getTerminator()) builder_.CreateBr(mergeBB);

    builder_.SetInsertPoint(mergeBB);
}

void LLVMVisitor::visit(NumberExpr* node) {
    lastValue_ = llvm::ConstantInt::get(
        llvm::Type::getInt32Ty(context_), node->getValue(), true);
}

void LLVMVisitor::visit(VariableExpr* node) {
    lastValue_ = loadVar(node->getName());
}

void LLVMVisitor::visit(BinaryExpr* node) {
    llvm::Value* lhs = evalExpr(node->getLeft());
    llvm::Value* rhs = evalExpr(node->getRight());
    switch (node->getOperator()) {
        case TokenType::PLUS:  lastValue_ = builder_.CreateAdd(lhs, rhs, "add"); break;
        case TokenType::MINUS: lastValue_ = builder_.CreateSub(lhs, rhs, "sub"); break;
        case TokenType::MUL:   lastValue_ = builder_.CreateMul(lhs, rhs, "mul"); break;
        case TokenType::DIV:   lastValue_ = builder_.CreateSDiv(lhs, rhs, "div"); break;
        default: throw std::runtime_error("CodegenError: unsupported binary operator");
    }
}

void LLVMVisitor::visit(EqualExpr* node) {
    lastValue_ = builder_.CreateZExt(
        builder_.CreateICmpEQ(evalExpr(node->getLeft()), evalExpr(node->getRight()), "eq"),
        llvm::Type::getInt32Ty(context_), "eqext");
}

void LLVMVisitor::visit(ClassDecl* node) {
    currentClass_ = node->getName();
    for (auto& mth : node->getMethods()) mth->accept(this);
    currentClass_.clear();
}

void LLVMVisitor::visit(FieldDecl* /*node*/) {}

void LLVMVisitor::visit(MethodDecl* node) {
    if (currentClass_.empty()) return;
    std::string mangledName = mangleMethod(currentClass_, node->getName());
    llvm::Function* fn = methodFunctions_[mangledName];
    if (!fn) return;

    builder_.SetInsertPoint(llvm::BasicBlock::Create(context_, "entry", fn));

    auto savedLocals = localVars_;
    auto savedThis   = thisPtr_;
    auto savedMethod = currentMethod_;
    localVars_.clear();
    currentMethod_ = node->getName();

    auto argIt = fn->arg_begin();
    thisPtr_ = &*argIt; thisPtr_->setName("self"); ++argIt;

    for (auto& param : node->getParams()) {
        llvm::AllocaInst* alloca = builder_.CreateAlloca(
            llvmTypeFor(param.type.typeName, param.type.isArray), nullptr, param.name);
        builder_.CreateStore(&*argIt, alloca);
        localVars_[param.name] = alloca;
        ++argIt;
    }

    for (auto& stmt : node->getBody()) stmt->accept(this);

    if (!builder_.GetInsertBlock()->getTerminator()) {
        llvm::Type* retTy = fn->getReturnType();
        if (retTy->isVoidTy())
            builder_.CreateRetVoid();
        else
            builder_.CreateRet(llvm::Constant::getNullValue(retTy));
    }

    llvm::verifyFunction(*fn);
    localVars_     = savedLocals;
    thisPtr_       = savedThis;
    currentMethod_ = savedMethod;
}

void LLVMVisitor::visit(ReturnStmt* node) {
    if (node->getExpression())
        builder_.CreateRet(evalExpr(node->getExpression()));
    else
        builder_.CreateRetVoid();
}

void LLVMVisitor::visit(VarDeclStmt* node) {
    llvm::Type* ty = llvmTypeFor(node->getType().typeName, node->getType().isArray);
    llvm::AllocaInst* alloca = builder_.CreateAlloca(ty, nullptr, node->getName());
    builder_.CreateStore(
        node->getInitializer() ? evalExpr(node->getInitializer())
                               : llvm::Constant::getNullValue(ty),
        alloca);
    localVars_[node->getName()] = alloca;
}

void LLVMVisitor::visit(NewObjectExpr* node) {
    auto it = classTypes_.find(node->getClassName());
    if (it == classTypes_.end())
        throw std::runtime_error("CodegenError: unknown class '" + node->getClassName() + "'");

    llvm::DataLayout dl(module_.get());
    llvm::Value* sizeVal = llvm::ConstantInt::get(
        llvm::Type::getInt64Ty(context_), dl.getTypeAllocSize(it->second));
    llvm::Value* rawPtr = builder_.CreateCall(mallocFunc_, {sizeVal}, "obj_raw");
    builder_.CreateMemSet(rawPtr,
        llvm::ConstantInt::get(llvm::Type::getInt8Ty(context_), 0),
        sizeVal, llvm::MaybeAlign(1));
    lastValue_ = builder_.CreateBitCast(
        rawPtr, llvm::PointerType::get(it->second, 0), node->getClassName() + "_ptr");
}

void LLVMVisitor::visit(NewArrayExpr* node) {
    llvm::Value* len64 = builder_.CreateZExt(
        evalExpr(node->getSize()), llvm::Type::getInt64Ty(context_), "len64");
    llvm::Value* bytes = builder_.CreateMul(
        len64, llvm::ConstantInt::get(llvm::Type::getInt64Ty(context_), 4), "bytes");
    lastValue_ = builder_.CreateBitCast(
        builder_.CreateCall(mallocFunc_, {bytes}, "arr_raw"),
        llvm::PointerType::get(llvm::Type::getInt32Ty(context_), 0), "arr_ptr");
}

void LLVMVisitor::visit(MethodCallExpr* node) {
    llvm::Value* objVal = evalExpr(node->getObject());

    std::string className;
    if (dynamic_cast<ThisExpr*>(node->getObject())) {
        className = currentClass_;
    } else {
        for (auto& [cn, cs] : symTable_.allClasses())
            if (cs.findMethod(node->getMethodName())) { className = cn; break; }
    }
    if (className.empty())
        throw std::runtime_error(
            "CodegenError: cannot resolve class for method '" + node->getMethodName() + "'");

    std::string mangledName = mangleMethod(className, node->getMethodName());
    llvm::Function* fn = methodFunctions_[mangledName];
    if (!fn)
        throw std::runtime_error("CodegenError: method not found: " + mangledName);

    std::vector<llvm::Value*> args;
    args.push_back(builder_.CreateBitCast(
        objVal, llvm::PointerType::get(classTypes_[className], 0), "self_cast"));
    for (auto& arg : node->getArgs()) args.push_back(evalExpr(arg.get()));
    lastValue_ = builder_.CreateCall(fn, args, "call_" + node->getMethodName());
}

void LLVMVisitor::visit(FieldAccessExpr* node) {
    llvm::Value* objVal = evalExpr(node->getObject());
    std::string className;
    for (auto& [cn, cs] : symTable_.allClasses())
        if (cs.findField(node->getFieldName())) { className = cn; break; }
    if (className.empty())
        throw std::runtime_error(
            "CodegenError: cannot resolve class for field '" + node->getFieldName() + "'");

    int idx = fieldIndex(className, node->getFieldName());
    if (idx < 0)
        throw std::runtime_error("CodegenError: field '" + node->getFieldName() + "' not found");

    llvm::Value* fieldPtr = builder_.CreateStructGEP(
        classTypes_[className],
        builder_.CreateBitCast(objVal, llvm::PointerType::get(classTypes_[className], 0)),
        static_cast<unsigned>(idx), node->getFieldName() + "_ptr");
    lastValue_ = builder_.CreateLoad(llvm::Type::getInt32Ty(context_), fieldPtr, node->getFieldName());
}

void LLVMVisitor::visit(ThisExpr* /*node*/) {
    if (!thisPtr_)
        throw std::runtime_error("CodegenError: 'this' used outside of a method");
    lastValue_ = thisPtr_;
}

void LLVMVisitor::visit(ArrayAccessExpr* node) {
    llvm::Value* elemPtr = builder_.CreateGEP(
        llvm::Type::getInt32Ty(context_), evalExpr(node->getArray()),
        evalExpr(node->getIndex()), "elem_ptr");
    lastValue_ = builder_.CreateLoad(llvm::Type::getInt32Ty(context_), elemPtr, "elem");
}

void LLVMVisitor::visit(ArrayLengthExpr* /*node*/) {
    lastValue_ = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), 0);
}

void LLVMVisitor::printIR() const {
    module_->print(llvm::outs(), nullptr);
}

void LLVMVisitor::saveIR(const std::string& filename) const {
    std::error_code ec;
    llvm::raw_fd_ostream out(filename, ec, llvm::sys::fs::OF_Text);
    if (ec)
        throw std::runtime_error(
            "CodegenError: cannot open '" + filename + "': " + ec.message());
    module_->print(out, nullptr);
}
