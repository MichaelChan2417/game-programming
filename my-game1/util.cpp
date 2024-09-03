#include "util.hpp"

#include <iostream>

void wait_and_exit(const std::string& message) {
    std::cerr << message << std::endl;
    wait_and_exit();
}

void wait_and_exit() {
    std::cout << "Press any to exit" << std::endl;
    std::cin.get();
    exit(-1);
}