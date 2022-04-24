#define CATCH_CONFIG_MAIN
#include "extern/catch2/catch.hpp"

#include <clapp.hpp>

TEST_CASE("test_int_store")
{
    std::vector<std::string> arguments{"", "-a", "123", "-b", "hello"};
    clapp::ArgumentParser parser(arguments);

    int out = 0;
    parser.option<int>("-a").store(out);
    parser.parse();

    REQUIRE(out == 123);
}

TEST_CASE("test_double_store")
{
    std::vector<std::string> arguments{"", "-a", "3.1415", "-b", "hello"};
    clapp::ArgumentParser parser(arguments);

    double out = 0;
    parser.option<double>("-a").store(out);
    parser.parse();

    REQUIRE(out == 3.1415);
}

TEST_CASE("test_string_store")
{
    std::vector<std::string> arguments{"", "-a", "something", "-b", "hello"};
    clapp::ArgumentParser parser(arguments);

    std::string out;
    parser.option<std::string>("-a").store(out);
    parser.parse();

    REQUIRE(out == "something");
}

TEST_CASE("test_value")
{
    std::vector<std::string> arguments{"", "-a", "something", "-b", "hello"};
    clapp::ArgumentParser parser(arguments);

    auto& option = parser.option<std::string>("-a");
    parser.parse();

    REQUIRE(option.value() == "something");
}

TEST_CASE("test_default_value")
{
    std::vector<std::string> arguments{"", "-x", "something", "-b", "hello"};
    clapp::ArgumentParser parser(arguments);

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