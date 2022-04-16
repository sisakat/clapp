# Clapp - Command Line Arguments++
A lightweight command line argument parsing library.

## Example
```cpp
#include <clapp.hpp>

int main(int argc, char* argv[])
{
    clapp::ArgumentParser parser(arguments);
    parser.name("Sample Application")
        .version("1.0.0")
        .description("Some really useful cli program.")
        .addHelp();

    std::string config;
    parser.option<std::string>("-c", "--cfg")
        .required()
        .argument("json config file")
        .store(config);

    bool silent;
    parser.option("-s")
        .description("Silent mode")
        .flag()
        .store(silent);
}
```