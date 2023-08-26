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

    std::string inputFilename;
    parser.option<std::string>("INPUT_FILENAME")
        .required()
        .description("Input filename.")
        .store(inputFilename);

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

    std::string loglevel;
    parser.option<std::string>("--loglevel")
        .choices({"trace", "debug", "info"})
        .description("Set the log level")
        .store(loglevel);

    if (parser.parse())
    {
        std::cout << "Input filename: " << inputFilename << std::endl;
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

./a.out [-h] INPUT_FILENAME [-v] -c <json config file> [-s] 
 [-f] [--loglevel debug|info|trace] 
-h  --help
    Print this help message.
INPUT_FILENAME
    Input filename.
-v --version
-c   --cfg <json config file>
    Sets the config file.
-s        
    Silent mode
-f        
    Flag for something.
--loglevel debug|info|trace
    Set the log level
```

```
> ./basic test.txt -c config.json -f
Input filename: test.txt
Config file: config.json
Silent mode set: 0
Flag set: 1
```

```
> ./basic -v
1.0
```