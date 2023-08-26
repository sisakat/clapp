#define CATCH_CONFIG_MAIN
#include "extern/catch2/catch.hpp"

#include <clapp.hpp>

TEST_CASE("test_int_store")
{
    std::vector<std::string> arguments{"", "-a", "123", "-b", "hello"};
    clapp::ArgumentParser parser(arguments);

    int out = 0;
    parser.option<int>("-a").store(out);
    parser.option<std::string>("-b");
    parser.parse();

    REQUIRE(out == 123);
}

TEST_CASE("test_double_store")
{
    std::vector<std::string> arguments{"", "-a", "3.1415", "-b", "hello"};
    clapp::ArgumentParser parser(arguments);

    double out = 0;
    parser.option<double>("-a").store(out);
    parser.option<std::string>("-b");
    parser.parse();

    REQUIRE(out == 3.1415);
}

TEST_CASE("test_string_store")
{
    std::vector<std::string> arguments{"", "-a", "something", "-b", "hello"};
    clapp::ArgumentParser parser(arguments);

    std::string out;
    parser.option<std::string>("-a").store(out);
    parser.option<std::string>("-b");
    parser.parse();

    REQUIRE(out == "something");
}

TEST_CASE("test_value")
{
    std::vector<std::string> arguments{"", "-a", "something", "-b", "hello"};
    clapp::ArgumentParser parser(arguments);

    auto& option = parser.option<std::string>("-a");
    parser.option<std::string>("-b");
    parser.parse();

    REQUIRE(option.value() == "something");
}

TEST_CASE("test_default_value")
{
    std::vector<std::string> arguments{"", "-x", "something", "-b", "hello"};
    clapp::ArgumentParser parser(arguments);

    parser.option<std::string>("-x");
    parser.option<std::string>("-b");
    auto& option = parser.option<std::string>("-a").defaultValue("abc");
    parser.parse();

    REQUIRE(option.value() == "abc");
}

TEST_CASE("test_required")
{
    std::vector<std::string> arguments{"", "-x", "something", "-b", "hello"};
    clapp::ArgumentParser parser(arguments);

    auto& option = parser.option<std::string>("-a").required();
    REQUIRE_THROWS(parser.parse());
}

TEST_CASE("test_multiple")
{
    std::vector<std::string> arguments{"", "-a", "-cfg", "config.json", "-d"};
    clapp::ArgumentParser parser(arguments);

    std::string cfg;
    parser.option("-a").required().flag();
    parser.option<std::string>("-cfg").required().store(cfg);
    auto& d = parser.option("-d").flag();

    REQUIRE_NOTHROW(parser.parse());
    REQUIRE(d.value());
    REQUIRE(cfg == "config.json");
}

TEST_CASE("test_no_argument_data")
{
    std::vector<std::string> arguments{"", "-a", "-cfg", "-d", "-d"};
    clapp::ArgumentParser parser(arguments);

    std::string cfg;
    parser.option("-a").required().flag();
    parser.option<std::string>("-cfg").required().store(cfg);
    parser.option("-d").flag();

    REQUIRE_THROWS(parser.parse());
}

TEST_CASE("test_option_with_equal_sign")
{
    std::vector<std::string> arguments{"", "--option=value", "-a"};
    clapp::ArgumentParser parser(arguments);

    std::string cfg;
    parser.option("-a").required().flag();
    parser.option<std::string>("--option").store(cfg);
    parser.parse();

    REQUIRE(cfg == "value");
}

TEST_CASE("test_option_with_choices")
{
    std::vector<std::string> arguments{"", "--option=value", "-a"};
    clapp::ArgumentParser parser(arguments);

    std::string cfg;
    parser.option<std::string>("--option")
        .choices({"value", "value1"})
        .store(cfg);
    parser.option("-a").flag();
    parser.parse();

    REQUIRE(cfg == "value");
}

TEST_CASE("test_option_with_choices_fail")
{
    std::vector<std::string> arguments{"", "--option=value2", "-a"};
    clapp::ArgumentParser parser(arguments);

    std::string cfg;
    parser.option<std::string>("--option")
        .choices({"value", "value1"})
        .store(cfg);

    REQUIRE_THROWS(parser.parse());
}

TEST_CASE("test_option_positional")
{
    std::vector<std::string> arguments{"", "file.txt"};
    clapp::ArgumentParser parser(arguments);

    auto& option = parser.option<std::string>("SOME_FILE");
    parser.parse();

    REQUIRE(option.value() == "file.txt");
}

TEST_CASE(
    "test_option_positional_with_other_arguments_after_positional_argument")
{
    std::vector<std::string> arguments{"", "file.txt", "-d", "1"};
    clapp::ArgumentParser parser(arguments);

    auto& positionalOption = parser.option<std::string>("SOME_FILE");
    auto& intOption = parser.option<int>("-d");
    parser.parse();

    REQUIRE(positionalOption.value() == "file.txt");
    REQUIRE(intOption.value() == 1);
}

TEST_CASE(
    "test_option_positional_with_other_arguments_before_positional_argument")
{
    std::vector<std::string> arguments{"", "-i", "1", "file.txt"};
    clapp::ArgumentParser parser(arguments);

    auto& positionalOption = parser.option<std::string>("SOME_FILE");
    auto& intOption = parser.option<int>("-i");
    parser.parse();

    REQUIRE(positionalOption.value() == "file.txt");
    REQUIRE(intOption.value() == 1);
}

TEST_CASE("test_option_multiple_positional")
{
    std::vector<std::string> arguments{"", "fileIn.txt", "fileOut.txt"};
    clapp::ArgumentParser parser(arguments);

    auto& positionalOptionIn = parser.option<std::string>("INPUT_FILE");
    auto& positionalOptionOut = parser.option<std::string>("OUTPUT_FILE");
    parser.parse();

    REQUIRE(positionalOptionIn.value() == "fileIn.txt");
    REQUIRE(positionalOptionOut.value() == "fileOut.txt");
}

TEST_CASE("test_option_multiple_positional_interleaved_with_other_arguments")
{
    std::vector<std::string> arguments{
        "", "fileIn.txt", "-i1", "1", "fileOut.txt", "-i2", "2"};
    clapp::ArgumentParser parser(arguments);

    auto& positionalOptionIn = parser.option<std::string>("INPUT_FILE");
    auto& positionalOptionOut = parser.option<std::string>("OUTPUT_FILE");
    auto& intOption1 = parser.option<int>("-i1");
    auto& intOption2 = parser.option<int>("-i2");
    parser.parse();

    REQUIRE(positionalOptionIn.value() == "fileIn.txt");
    REQUIRE(positionalOptionOut.value() == "fileOut.txt");
    REQUIRE(intOption1.value() == 1);
    REQUIRE(intOption2.value() == 2);
}

TEST_CASE("test_option_multiple_positional_with_required_arguments")
{
    std::vector<std::string> arguments{"", "fileIn.txt"};
    clapp::ArgumentParser parser(arguments);

    parser.option<std::string>("INPUT_FILE");
    parser.option<int>("-i").required();

    REQUIRE_THROWS(parser.parse());
}

TEST_CASE("test_option_multiple_positional_required")
{
    std::vector<std::string> arguments{"", "-a", "test"};
    clapp::ArgumentParser parser(arguments);

    parser.option<std::string>("INPUT_FILE").required();

    REQUIRE_THROWS(parser.parse());
}

TEST_CASE("test_option_multiple_positional_required_empty")
{
    std::vector<std::string> arguments{""};
    clapp::ArgumentParser parser(arguments);

    parser.option<std::string>("INPUT_FILE").required();

    REQUIRE_FALSE(parser.parse());
}
