#include <iostream>

int main()
{
    const uint32_t* address = (uint32_t*)0x1234;
    std::cout << *address;
    return 0;
}
