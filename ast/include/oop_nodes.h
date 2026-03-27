#pragma once
#include "node.h"
#include "expression.h"
#include "symbol_table.h"
#include <string>
#include <vector>
#include <memory>

struct TypeAnnotation {
    std::string typeName;
    bool        isArray = false;

    TypeInfo toTypeInfo() const {
        if (typeName == "int") {
            return isArray ? TypeInfo::makeArrayInt() : TypeInfo::makeInt();
        }
        return isArray ? TypeInfo::makeArrayClass(typeName)
                       : TypeInfo::makeClass(typeName);
    }
};

struct FormalParam {
    TypeAnnotation type;
    std::string    name;
};

class FieldDecl : public Node {
    TypeAnnotation type_;
    std::string    name_;
public:
    FieldDecl(TypeAnnotation t, std::string n)
        : type_(std::move(t)), name_(std::move(n)) {}

    void execute(Context& ctx) override {}
    void accept(Visitor* visitor) override;

    const TypeAnnotation& getType() const { return type_; }
    const std::string&    getName() const { return name_; }
};

class MethodDecl : public Node {
    TypeAnnotation              returnType_;
    std::string                 name_;
    std::vector<FormalParam>    params_;
    std::vector<std::unique_ptr<Node>> body_;
public:
    MethodDecl(TypeAnnotation ret, std::string n, std::vector<FormalParam> params)
        : returnType_(std::move(ret)), name_(std::move(n)), params_(std::move(params)) {}

    void execute(Context& ctx) override {}
    void accept(Visitor* visitor) override;

    const TypeAnnotation&              getReturnType() const { return returnType_; }
    const std::string&                 getName()       const { return name_; }
    const std::vector<FormalParam>&    getParams()     const { return params_; }
    std::vector<std::unique_ptr<Node>>& getBody()             { return body_; }
    const std::vector<std::unique_ptr<Node>>& getBody() const { return body_; }

    void addStatement(std::unique_ptr<Node> stmt) {
        body_.push_back(std::move(stmt));
    }
};

class ClassDecl : public Node {
    std::string                         name_;
    std::vector<std::unique_ptr<FieldDecl>>  fields_;
    std::vector<std::unique_ptr<MethodDecl>> methods_;
public:
    explicit ClassDecl(std::string n) : name_(std::move(n)) {}

    void execute(Context& ctx) override {}
    void accept(Visitor* visitor) override;

    const std::string& getName() const { return name_; }

    const std::vector<std::unique_ptr<FieldDecl>>&  getFields()  const { return fields_; }
    const std::vector<std::unique_ptr<MethodDecl>>& getMethods() const { return methods_; }

    void addField (std::unique_ptr<FieldDecl>  f) { fields_.push_back(std::move(f)); }
    void addMethod(std::unique_ptr<MethodDecl> m) { methods_.push_back(std::move(m)); }
};

class ReturnStmt : public Node {
    std::unique_ptr<Expression> expr_;
public:
    explicit ReturnStmt(std::unique_ptr<Expression> e = nullptr)
        : expr_(std::move(e)) {}

    void execute(Context& ctx) override {}
    void accept(Visitor* visitor) override;

    Expression* getExpression() { return expr_.get(); }
    const Expression* getExpression() const { return expr_.get(); }
};

class NewObjectExpr : public Expression {
    std::string                         className_;
    std::vector<std::unique_ptr<Expression>> args_;
public:
    explicit NewObjectExpr(std::string cls)
        : className_(std::move(cls)) {}

    int evaluate(Context& ctx) override { return 0; }
    void accept(Visitor* visitor) override;

    const std::string& getClassName() const { return className_; }
    const std::vector<std::unique_ptr<Expression>>& getArgs() const { return args_; }
    std::vector<std::unique_ptr<Expression>>& getArgs() { return args_; }

    void addArg(std::unique_ptr<Expression> a) { args_.push_back(std::move(a)); }
};

class NewArrayExpr : public Expression {
    TypeAnnotation              elemType_;
    std::unique_ptr<Expression> size_;
public:
    NewArrayExpr(TypeAnnotation t, std::unique_ptr<Expression> sz)
        : elemType_(std::move(t)), size_(std::move(sz)) {}

    int evaluate(Context& ctx) override { return 0; }
    void accept(Visitor* visitor) override;

    const TypeAnnotation& getElemType() const { return elemType_; }
    Expression*           getSize()           { return size_.get(); }
    const Expression*     getSize()     const { return size_.get(); }
};

class MethodCallExpr : public Expression {
    std::unique_ptr<Expression>              object_;
    std::string                              methodName_;
    std::vector<std::unique_ptr<Expression>> args_;
public:
    MethodCallExpr(std::unique_ptr<Expression> obj, std::string method)
        : object_(std::move(obj)), methodName_(std::move(method)) {}

    int evaluate(Context& ctx) override { return 0; }
    void accept(Visitor* visitor) override;

    Expression*        getObject()     { return object_.get(); }
    const Expression*  getObject()     const { return object_.get(); }
    const std::string& getMethodName() const { return methodName_; }
    const std::vector<std::unique_ptr<Expression>>& getArgs() const { return args_; }
    std::vector<std::unique_ptr<Expression>>& getArgs() { return args_; }

    void addArg(std::unique_ptr<Expression> a) { args_.push_back(std::move(a)); }
};

class FieldAccessExpr : public Expression {
    std::unique_ptr<Expression> object_;
    std::string                 fieldName_;
public:
    FieldAccessExpr(std::unique_ptr<Expression> obj, std::string field)
        : object_(std::move(obj)), fieldName_(std::move(field)) {}

    int evaluate(Context& ctx) override { return 0; }
    void accept(Visitor* visitor) override;

    Expression*        getObject()    { return object_.get(); }
    const Expression*  getObject()    const { return object_.get(); }
    const std::string& getFieldName() const { return fieldName_; }
};

class ThisExpr : public Expression {
public:
    int evaluate(Context& ctx) override { return 0; }
    void accept(Visitor* visitor) override;
};

class ArrayAccessExpr : public Expression {
    std::unique_ptr<Expression> array_;
    std::unique_ptr<Expression> index_;
public:
    ArrayAccessExpr(std::unique_ptr<Expression> arr, std::unique_ptr<Expression> idx)
        : array_(std::move(arr)), index_(std::move(idx)) {}

    int evaluate(Context& ctx) override { return 0; }
    void accept(Visitor* visitor) override;

    Expression* getArray() { return array_.get(); }
    Expression* getIndex() { return index_.get(); }
};

class ArrayLengthExpr : public Expression {
    std::unique_ptr<Expression> array_;
public:
    explicit ArrayLengthExpr(std::unique_ptr<Expression> arr)
        : array_(std::move(arr)) {}

    int evaluate(Context& ctx) override { return 0; }
    void accept(Visitor* visitor) override;

    Expression* getArray() { return array_.get(); }
};

class VarDeclStmt : public Node {
    TypeAnnotation              type_;
    std::string                 name_;
    std::unique_ptr<Expression> init_;
public:
    VarDeclStmt(TypeAnnotation t, std::string n,
                std::unique_ptr<Expression> init = nullptr)
        : type_(std::move(t)), name_(std::move(n)), init_(std::move(init)) {}

    void execute(Context& ctx) override {}
    void accept(Visitor* visitor) override;

    const TypeAnnotation& getType()        const { return type_; }
    const std::string&    getName()        const { return name_; }
    Expression*           getInitializer()       { return init_.get(); }
    const Expression*     getInitializer() const { return init_.get(); }
};
