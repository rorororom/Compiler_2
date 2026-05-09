#pragma once
#include "node.h"
#include <vector>
#include <memory>

class ClassDecl;

class Program : public Node {
public:
    std::vector<std::unique_ptr<ClassDecl>> classes;
    std::vector<std::unique_ptr<Node>> statements;

    void execute(Context& ctx) override {
        for (auto& stmt : statements)
            stmt->execute(ctx);
    }
    
    void accept(Visitor* visitor) override;
    
    const std::vector<std::unique_ptr<ClassDecl>>& getClasses() const {
        return classes;
    }

    const std::vector<std::unique_ptr<Node>>& getStatements() const {
        return statements;
    }
};