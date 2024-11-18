// Main.cpp
#include <iostream>
#include "Core/Debug/Log.h"

int main()
{
    NES_INIT_LOGGER("/Log");
    NES_LOG("Test", "Hello, World!");
    NES_WARN("This is a warning!");
    NES_ERROR("This is an error!");
    NES_CLOSE_LOGGER();
    return 0;
}