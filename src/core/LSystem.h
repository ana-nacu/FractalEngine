#pragma once

#include <string>
#include <unordered_map>

class LSystem {
public:
    std::string axiom;  // Regula inițială
    std::unordered_map<char, std::string> rules;  // Reguli de producție
    int iterations;  // Numărul de iterații aplicate

    LSystem(std::string axiom, int iterations);
    
    void addRule(char predecessor, std::string successor);
    std::string generate();
};