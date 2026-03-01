#pragma once
#include <vector>
#include <memory>

class Program : public Node {
public:
    std::vector<std::unique_ptr<Node>> statements;

    void execute(Context& ctx) override {
        for (auto& stmt : statements)
            stmt->execute(ctx);
    }
};