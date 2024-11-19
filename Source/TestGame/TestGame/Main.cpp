// Main.cpp
#include <Nessie.h>

#if NES_LOGGING_ENABLED
void Foo() {}
#endif

int main()
{
    NES_INIT_LOGGER("/Log");
    NES_LOGV("Test", "Hello, World!");
    NES_WARN("This is a warning!");
    NES_ERROR("This is an error!");
    NES_CLOSE_LOGGER();

    return 0;
}