#pragma once

//4.1.7, p10
#include <cstdint>
#include <cmath>
#include <stdexcept>
#include <iostream>

struct Hybrid {
    void set(unsigned int i);  // exception if too big or negative (or assert)
    unsigned int get();  // should always work
    unsigned int get_int();  // works only if the byte is actually an int, otherwise an exception
private:
    uint8_t byte;
};
