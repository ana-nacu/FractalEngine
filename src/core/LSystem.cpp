#include "LSystem.h"

LSystem::LSystem(std::string axiom, int iterations)
    : axiom(axiom), iterations(iterations) {}

void LSystem::addRule(char predecessor, std::string successor) {
    rules[predecessor] = successor;
}

std::string LSystem::generate() {
    std::string current = axiom;

    for (int i = 0; i < iterations; i++) {
        std::string next;
        for (char c : current) {
            if (rules.find(c) != rules.end()) {
                next += rules.at(c);  // Înlocuim simbolul conform regulilor
            } else {
                next += c;  // Dacă nu există o regulă, păstrăm simbolul
            }
        }
        current = next;
    }
    
    return current;
}