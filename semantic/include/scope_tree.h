#pragma once
#include "symbol_table.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <iostream>

enum class ScopeKind {
    GLOBAL,
    CLASS,
    METHOD,
    BLOCK
};

inline std::string scopeKindName(ScopeKind k) {
    switch (k) {
        case ScopeKind::GLOBAL: return "global";
        case ScopeKind::CLASS:  return "class";
        case ScopeKind::METHOD: return "method";
        case ScopeKind::BLOCK:  return "block";
    }
    return "?";
}

class Scope {
public:
    explicit Scope(ScopeKind kind,
                   Scope*     parent  = nullptr,
                   std::string label  = "")
        : kind_(kind), parent_(parent), label_(std::move(label)) {}

    Scope*       parent()       { return parent_; }
    const Scope* parent() const { return parent_; }

    Scope* addChild(std::unique_ptr<Scope> child) {
        children_.push_back(std::move(child));
        return children_.back().get();
    }

    const std::vector<std::unique_ptr<Scope>>& children() const { return children_; }

    ScopeKind          kind()  const { return kind_; }
    const std::string& label() const { return label_; }

    void setClassSymbol(const ClassSymbol* cs)   { classSymbol_ = cs; }
    const ClassSymbol* classSymbol()       const  { return classSymbol_; }

    void setMethodSymbol(const MethodSymbol* ms) { methodSymbol_ = ms; }
    const MethodSymbol* methodSymbol()     const  { return methodSymbol_; }

    void declare(const VariableSymbol& sym, bool warnShadow = true) {
        if (locals_.count(sym.name)) {
            throw std::runtime_error(
                "SemanticError: variable '" + sym.name +
                "' is already declared in this scope" +
                (label_.empty() ? "" : " (scope: " + label_ + ")"));
        }
        if (warnShadow) {
            const Scope* outer = parent_;
            while (outer) {
                if (outer->locals_.count(sym.name)) {
                    std::cerr << "Warning: variable '" << sym.name
                              << "' shadows an outer declaration"
                              << (label_.empty() ? "" : " in scope '" + label_ + "'")
                              << "\n";
                    break;
                }
                outer = outer->parent_;
            }
        }
        locals_.emplace(sym.name, sym);
    }

    const VariableSymbol* lookup(const std::string& name) const {
        auto it = locals_.find(name);
        if (it != locals_.end()) return &it->second;
        if (parent_) return parent_->lookup(name);
        return nullptr;
    }

    const VariableSymbol* lookupLocal(const std::string& name) const {
        auto it = locals_.find(name);
        return it != locals_.end() ? &it->second : nullptr;
    }

    const std::unordered_map<std::string, VariableSymbol>& locals() const {
        return locals_;
    }

    void dump(std::ostream& os, int depth = 0) const {
        std::string pad(depth * 2, ' ');
        os << pad << "[" << scopeKindName(kind_);
        if (!label_.empty()) os << " '" << label_ << "'";
        os << "]\n";
        for (auto& [name, sym] : locals_) {
            os << pad << "  var " << name << " : " << sym.type.toString();
            switch (sym.storage) {
                case VariableSymbol::StorageKind::LOCAL: os << " (local)";  break;
                case VariableSymbol::StorageKind::FIELD: os << " (field)";  break;
                case VariableSymbol::StorageKind::PARAM: os << " (param)";  break;
            }
            os << "\n";
        }
        for (auto& child : children_)
            child->dump(os, depth + 1);
    }

private:
    ScopeKind   kind_;
    Scope*      parent_;
    std::string label_;

    const ClassSymbol*  classSymbol_  = nullptr;
    const MethodSymbol* methodSymbol_ = nullptr;

    std::unordered_map<std::string, VariableSymbol> locals_;
    std::vector<std::unique_ptr<Scope>> children_;
};

class ScopeTree {
public:
    ScopeTree() {
        root_ = std::make_unique<Scope>(ScopeKind::GLOBAL, nullptr, "global");
        current_ = root_.get();
    }

    Scope* pushScope(ScopeKind kind, const std::string& label = "") {
        auto child = std::make_unique<Scope>(kind, current_, label);
        Scope* raw = current_->addChild(std::move(child));
        current_ = raw;
        return raw;
    }

    void popScope() {
        if (!current_->parent())
            throw std::logic_error("ScopeTree: cannot pop the root scope");
        current_ = current_->parent();
    }

    Scope*       current()       { return current_; }
    const Scope* current() const { return current_; }

    Scope*       root()          { return root_.get(); }
    const Scope* root()    const { return root_.get(); }

    void declare(const VariableSymbol& sym, bool warnShadow = true) {
        current_->declare(sym, warnShadow);
    }

    const VariableSymbol* lookup(const std::string& name) const {
        const VariableSymbol* sym = current_->lookup(name);
        if (!sym)
            throw std::runtime_error(
                "SemanticError: variable '" + name + "' used before declaration");
        return sym;
    }

    void dump(std::ostream& os) const {
        root_->dump(os, 0);
    }

private:
    std::unique_ptr<Scope> root_;
    Scope*                 current_;
};
