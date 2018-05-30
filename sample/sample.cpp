#include <toml/toml.hpp>
#include <iostream>

int main(int argc, char ** argv)
{
    if(argc != 2)
    {
        std::cerr << "usage: ./sample [filename]" << std::endl;
        return 1;
    }

    std::cout << toml::parse(std::string(argv[1])) << std::endl;

    return 0;
}
