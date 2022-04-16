# Clapp - Command Line Argument Parser++
A lightweight header only command line argument parsing library. Just download [clapp.hpp](include/clapp.hpp) and get to work.

## Example
```cpp
#include "../include/clapp.hpp"
#include <iostream>

int main(int argc, char* argv[])
{
    clapp::ArgumentParser parser(argc, argv);
    parser.name("Sample Application")
        .version("1.0.0")
        .description("Some really useful cli program.")
        .addHelp();

    parser.option("-v", "--version")
        .flag()
        .overruling()
        .callback([](auto val) {
            std::cout << "1.0" << std::endl;
        });

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
```

```
> ./basic -h
Sample Application 1.0.0
Some really useful cli program.

./basic [-h] [-v] -c <json config file> [-s] 
 [-f] 
-h  --help
    Print this help message.
-v --version
-c   --cfg <json config file>
    Sets the config file.
-s        
    Silent mode
-f        
    Flag for something.
```

```
> ./basic -c config.json -f
Config file: config.json
Silent mode set: 0
Flag set: 1
```

```
> ./basic -v
1.0
```