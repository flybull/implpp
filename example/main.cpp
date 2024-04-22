#include <iostream>
#include "main.hpp"

void Initializer::on_begin(const std::string& name)
{
    std::cout << "on_begin,name=" << name << "\n";
}
void Initializer::on_success(const std::string& name)
{
    std::cout << "on_success,name=" << name << "\n";
}
void Initializer::on_error(const std::string& name)
{
    std::cout << "on_error,name=" << name << "\n";
}

int main()
{
    Initializer::instance().init();
    return 0;
}