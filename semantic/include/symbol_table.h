#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <stdexcept>

enum class VarKind {
    INT,
    CLASS_INST,
    ARRAY_INT,
    ARRAY_CLASS
};

struct TypeInfo {
    VarKind kind      = VarKind::INT;
    std::string className;

    static TypeInfo makeInt()  { return {VarKind::INT, ""}; }
    static TypeInfo makeClass(const std::string& name) {
        return {VarKind::CLASS_INST, name};
    }
    static TypeInfo makeArrayInt()  { return {VarKind::ARRAY_INT, ""}; }
    static TypeInfo makeArrayClass(const std::string& name) {
        return {VarKind::ARRAY_CLASS, name};
    }

    std::string toString() const {
        switch (kind) {
            case VarKind::INT:         return "int";
            case VarKind::CLASS_INST:  return className;
            case VarKind::ARRAY_INT:   return "int[]";
            case VarKind::ARRAY_CLASS: return className + "[]";
        }
        return "unknown";
    }
};

struct VariableSymbol {
    std::string name;
    TypeInfo    type;

    enum class StorageKind { LOCAL, FIELD, PARAM } storage = StorageKind::LOCAL;

    int index = -1;

    VariableSymbol() = default;
    VariableSymbol(std::string n, TypeInfo t, StorageKind sk = StorageKind::LOCAL)
        : name(std::move(n)), type(std::move(t)), storage(sk) {}
};

struct MethodSymbol {
    std::string name;
    TypeInfo    returnType;
    std::vector<VariableSymbol> params;
    std::string ownerClass;
    bool isConstructor = false;

    MethodSymbol() = default;
    MethodSymbol(std::string n, TypeInfo ret, std::string owner, bool isCtor = false)
        : name(std::move(n)), returnType(std::move(ret)),
          ownerClass(std::move(owner)), isConstructor(isCtor) {}

    void addParam(VariableSymbol p) { params.push_back(std::move(p)); }

    const VariableSymbol* findParam(const std::string& pname) const {
        for (auto& p : params)
            if (p.name == pname) return &p;
        return nullptr;
    }
};

struct ClassSymbol {
    std::string name;

    std::vector<VariableSymbol>                          fields;
    std::unordered_map<std::string, MethodSymbol>        methods;

    ClassSymbol() = default;
    explicit ClassSymbol(std::string n) : name(std::move(n)) {}

    void addField(VariableSymbol f) {
        f.storage = VariableSymbol::StorageKind::FIELD;
        f.index   = static_cast<int>(fields.size());
        fields.push_back(std::move(f));
    }

    const VariableSymbol* findField(const std::string& fname) const {
        for (auto& f : fields)
            if (f.name == fname) return &f;
        return nullptr;
    }

    void addMethod(MethodSymbol m) {
        std::string key = m.name;
        methods.emplace(std::move(key), std::move(m));
    }

    const MethodSymbol* findMethod(const std::string& mname) const {
        auto it = methods.find(mname);
        return it != methods.end() ? &it->second : nullptr;
    }

    void ensureConstructor() {
        if (methods.count(name) == 0) {
            MethodSymbol ctor(name, TypeInfo::makeClass(name), name, true);
            methods.emplace(name, std::move(ctor));
        }
    }

    std::size_t fieldCount() const { return fields.size(); }
};

class SymbolTable {
public:
    void addClass(ClassSymbol cls) {
        if (classes_.count(cls.name))
            throw std::runtime_error("Class '" + cls.name + "' already defined");
        std::string key = cls.name;
        classes_.emplace(std::move(key), std::move(cls));
    }

    const ClassSymbol* findClass(const std::string& name) const {
        auto it = classes_.find(name);
        return it != classes_.end() ? &it->second : nullptr;
    }

    ClassSymbol* findClassMut(const std::string& name) {
        auto it = classes_.find(name);
        return it != classes_.end() ? &it->second : nullptr;
    }

    const std::unordered_map<std::string, ClassSymbol>& allClasses() const {
        return classes_;
    }

private:
    std::unordered_map<std::string, ClassSymbol> classes_;
};
