#pragma once
#include <unordered_map>
#include <string>
#include <iostream>

class Context {
public:
    std::unordered_map<std::string, int> variables;

    void declare(const std::string& name) {
        variables[name] = 0;
    }

    void assign(const std::string& name, int value) {
        variables[name] = value;
    }

    int get(const std::string& name) {
        return variables[name];
    }

    void print(int value) {
        std::cout << value << std::endl;
    }
};