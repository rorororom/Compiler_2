#pragma once
#include <unordered_map>
#include <string>
#include <iostream>

#include <vector>

class Context {
public:
    std::vector<std::unordered_map<std::string, int>> scopes;

    Context() {
        scopes.push_back({});
    }

    void enterScope() {
        scopes.push_back({});
    }

    void exitScope() {
        if (scopes.size() > 1) {
            scopes.pop_back();
        }
    }

    void declare(const std::string& name) {
        scopes.back()[name] = 0;
    }

    void assign(const std::string& name, int value) {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            if (it->find(name) != it->end()) {
                (*it)[name] = value;
                return;
            }
        }
        scopes.back()[name] = value;
    }

    int get(const std::string& name) {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            if (it->find(name) != it->end()) {
                return (*it)[name];
            }
        }
        return 0;
    }

    void print(int value) {
        std::cout << value << std::endl;
    }
};