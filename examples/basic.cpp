#include "../include/clapp.hpp"
#include <iostream>

int main(int argc, char* argv[])
{
    clapp::ArgumentParser parser(argc, argv);
    parser.name("Sample Application")
        .version("1.0.0")
        .description("Some really useful cli program.")
        .addHelp();

    std::string config;
    parser.option<std::string>("-c", "--cfg")
        .required()
        .argument("json config file")
        .description("Sets the config file.")
        .store(config);

    bool silent = false;
    parser.option("-s")
        .description("Silent mode")
        .flag()
        .store(silent);

    auto& flag = parser.option("-f")
        .flag()
        .description("Flag for something.");

    if (parser.parse())
    {
        std::cout << "Config file: " << config << std::endl;
        std::cout << "Silent mode set: " << silent << std::endl;
        std::cout << "Flag set: " << flag.value() << std::endl;
    }
}