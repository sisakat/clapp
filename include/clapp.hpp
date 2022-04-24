/*
   ___ _      _   ___ ___
  / __| |    /_\ | _ \ _ \  Command Line Argument Parser++
 | (__| |__ / _ \|  _/  _/  Version 1.1.0
  \___|____/_/ \_\_| |_|    https://github.com/sisakat/clapp

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2022 Stefan Isak <http://sisak.at>.

Permission is hereby  granted, free of charge, to any  person obtaining a copy
of this software and associated  documentation files (the "Software"), to deal
in the Software  without restriction, including without  limitation the rights
to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <cassert>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#define CLAPP_VERSION_MAJOR 1
#define CLAPP_VERSION_MINOR 1
#define CLAPP_VERSION_PATCH 0

namespace clapp
{

/* Type conversions */

template <typename T> struct TypeParser
{
    static T Get(std::string value) { return T{value}; }
};

template <> struct TypeParser<int>
{
    static int Get(std::string value) { return std::stoi(value); }
};

template <> struct TypeParser<double>
{
    static double Get(std::string value) { return std::stod(value); }
};

template <> struct TypeParser<float>
{
    static float Get(std::string value) { return std::stof(value); }
};

template <> struct TypeParser<bool>
{
    static bool Get(std::string value)
    {
        return value.empty() || value == "1" || value == "true";
    }
};

/* Argument parser */

class ArgumentParser
{
public:
    /**
     * @brief Exception used for errors during the command line parsing. For
     * example if an option is missing but required.
     *
     */
    class ArgumentParserException : public std::runtime_error
    {
    public:
        ArgumentParserException(const std::string& what)
            : std::runtime_error(what)
        {
        }
    };

    /**
     * @brief Option class representing a command line option the user can
     * specify. Use for internal purspose only.
     *
     */
    struct Option
    {
        virtual ~Option() = default;
        friend class ArgumentParser;

    protected:
        Option(const std::string& _short_option,
               const std::string& _long_option)
            : short_option{_short_option}, long_option{_long_option}
        {
        }
        Option(const std::string& long_option) : Option("", long_option) {}
        virtual void setValue(const std::string& value) = 0;
        virtual void invokeCallback() = 0;

        bool operator<(const Option& other) { return name() < other.name(); }

        std::string name() const
        {
            std::stringstream ss;
            ss << short_option;
            if (!long_option.empty())
            {
                ss << " (" << long_option << ")";
            }
            return ss.str();
        }

        std::string argument_name;
        std::string short_option;
        std::string long_option;
        std::string description;

        bool required = false;
        bool set = false;
        bool flag = false;
        bool overruling = false;
        bool has_default_value = false;
    };

    /**
     * @brief Wrapper of an user specified option that can be interacted with.
     * To configure the option use the public member functions of this class.
     *
     * @tparam T
     */
    template <typename T> class OptionWrapper : public Option
    {
    public:
        OptionWrapper(const std::string& short_option,
                      const std::string& long_option)
            : Option(short_option, long_option)
        {
        }

        OptionWrapper(const std::string& long_option) : Option(long_option) {}

        OptionWrapper(const OptionWrapper&) = delete;
        virtual ~OptionWrapper() = default;

        /**
         * @brief Argument name.
         *
         * @param name Argument name displayed in the help message.
         * @return OptionWrapper<T>&
         */
        OptionWrapper<T>& argument(const std::string& name)
        {
            Option::argument_name = name;
            return *this;
        }

        /**
         * @brief Describes what the option does. Displayed in the help message.
         *
         * @param description Description of the option.
         */
        OptionWrapper<T>& description(const std::string& description)
        {
            Option::description = description;
            return *this;
        }

        /**
         * @brief Option must not be specified.
         *
         */
        OptionWrapper<T>& optional()
        {
            Option::required = false;
            return *this;
        }

        /**
         * @brief Option must be specified.
         *
         */
        OptionWrapper<T>& required()
        {
            Option::required = true;
            return *this;
        }

        /**
         * @brief Where the value is stored if it is provided.
         *
         * @param store Storage
         */
        OptionWrapper<T>& store(T& store)
        {
            m_ref = &store;
            return *this;
        }

        /**
         * @brief Function to be called if a value has been stored.
         *
         * @param callback Callback function
         */
        OptionWrapper<T>& callback(std::function<void(T)> callback)
        {
            m_callback = callback;
            return *this;
        }

        /**
         * @brief Option has no arguments and is treated as a flag.
         *
         */
        OptionWrapper<T>& flag()
        {
            Option::flag = true;
            return *this;
        }

        /**
         * @brief Default value if the option is not specified.
         *
         * @param value Some default value.
         * @return OptionWrapper<T>&
         */
        OptionWrapper<T>& defaultValue(T value)
        {
            Option::set = true;
            Option::has_default_value = true;
            m_value = value;
            return *this;
        }

        OptionWrapper<T>& overruling()
        {
            Option::overruling = true;
            return *this;
        }

        /**
         * @brief Current value stored in the option.
         *
         * @return T&
         */
        T& value() { return m_value; }

    private:
        friend class ArgumentParser;
        std::function<void(T)> m_callback = [](T val) {};
        T m_value;
        T* m_ref{nullptr};

        void setValue(const std::string& value) override
        {
            Option::set = true;
            m_value = TypeParser<T>::Get(value);
            if (m_ref)
                *m_ref = m_value;
        }

        void invokeCallback() override { m_callback(m_value); }
    };

    ArgumentParser(int argc, char* argv[]) : m_argv{argv, argv + argc} {}
    ArgumentParser(const std::vector<std::string>& arguments)
        : m_argv{arguments}
    {
    }

    /**
     * @brief Parses the arguments, stores the values and invokes callbacks.
     *
     */
    bool parse()
    {
        if (m_argv.size() < 2)
        {
            printHelp();
            return false;
        }

        parseArguments();

        if (checkOverrulingOptions())
        {
            return false;
        }

        checkRequiredOptions();
        invokeCallbacks();
        return true;
    }

    /**
     * @brief Option that stores a T value.
     *
     * @tparam T Some type that can be constructed by a string or has a defined
     * TypeParser.
     * @param short_option Short name of the option.
     * @param long_option Long name of the option.
     * @return OptionWrapper<T>&
     */
    template <typename T>
    OptionWrapper<T>& option(const std::string& short_option,
                             const std::string& long_option)
    {
        m_options.emplace_back(
            std::make_unique<OptionWrapper<T>>(short_option, long_option));
        auto& option_ptr = m_options.back();

        if (short_option.empty() && long_option.empty())
        {
            throw ArgumentParserException(
                "Short option and long option name cannot both be empty.");
        }

        if (!short_option.empty())
        {

            m_options_map[short_option] = m_options.size() - 1;
        }

        if (!long_option.empty())
        {
            m_options_map[long_option] = m_options.size() - 1;
        }

        return *(reinterpret_cast<OptionWrapper<T>*>(option_ptr.get()));
    }

    /**
     * @brief Option that stores a T value.
     *
     * @tparam T Some type that can be constructed by a string or has a defined
     * TypeParser.
     * @param long_option Long name of the option.
     * @return OptionWrapper<T>&
     */
    template <typename T>
    OptionWrapper<T>& option(const std::string& long_option)
    {
        return option<T>({}, long_option);
    }

    /**
     * @brief Option that acts like a flag and stores a boolean.
     *
     * @param short_option Short name of the option.
     * @param long_option Long name of the option.
     * @return OptionWrapper<bool>&
     */
    OptionWrapper<bool>& option(const std::string& short_option,
                                const std::string& long_option)
    {
        return option<bool>(short_option, long_option);
    }

    /**
     * @brief Option that acts like a flag and stores a boolean.
     *
     * @param long_option Long name of the option.
     * @return OptionWrapper<bool>&
     */
    OptionWrapper<bool>& option(const std::string& long_option)
    {
        return option<bool>({}, long_option);
    }

    /**
     * @brief Returns the help message containing the name and description of
     * each option.
     *
     * @return std::string
     */
    std::string help() const
    {
        std::stringstream ss;
        if (!m_name.empty())
        {
            ss << m_name;
            if (m_version.empty())
            {
                ss << std::endl;
            }
        }

        if (!m_version.empty())
        {
            ss << " " << m_version;
            ss << std::endl;
        }

        if (!m_description.empty())
        {
            ss << m_description << std::endl;
        }

        if (!m_name.empty() || !m_version.empty() || !m_description.empty())
        {
            ss << std::endl;
        }

        uint32_t size = ss.str().size();
        uint32_t line_length = size;
        ss << m_argv[0] << " ";
        for (const auto& option : m_options)
        {
            if (line_length > size + 100)
            {
                ss << std::endl << " ";
                line_length = size;
            }
            assert(!option->short_option.empty() ||
                   !option->long_option.empty());
            if (!option->required)
            {
                ss << "[";
            }
            if (!option->short_option.empty())
            {
                ss << option->short_option;
            }
            else if (!option->long_option.empty())
            {
                ss << option->long_option;
            }
            if (!option->argument_name.empty())
            {
                ss << " <" << option->argument_name << ">";
            }
            if (!option->required)
            {
                ss << "]";
            }
            ss << " ";
            line_length += ss.str().size() - size;
        }

        ss << std::endl;

        for (const auto& option : m_options)
        {
            if (!option->short_option.empty())
            {
                ss << std::setw(2) << std::left << option->short_option;
            }

            if (!option->long_option.empty())
            {
                if (option->short_option.empty())
                {
                    ss << std::setw(10) << std::left << option->long_option;
                }
                else
                {
                    ss << " " << std::right << std::setw(7)
                       << option->long_option;
                }
            }

            if (!option->argument_name.empty())
            {
                ss << " <" << option->argument_name << ">";
            }

            ss << std::right;
            if (!option->description.empty())
            {
                ss << std::endl;
                ss << "    " << option->description;
            }
            ss << std::endl;
        }
        return ss.str();
    }

    /**
     * @brief Prints the help message containing the name and description of
     * each option.
     *
     */
    void printHelp() const { std::cout << help(); }

    /**
     * @brief Adds a default option -h (--help).
     *
     * @return OptionWrapper<bool>&
     */
    auto& addHelp()
    {
        return this->option("-h", "--help")
            .flag()
            .overruling()
            .description("Print this help message.")
            .callback([this](auto) { this->printHelp(); });
    }

    /**
     * @brief Sets the name of the executing program. Displayed in the help
     * message.
     *
     * @param name Name string of the program.
     * @return ArgumentParser&
     */
    ArgumentParser& name(const std::string& name)
    {
        m_name = name;
        return *this;
    }

    /**
     * @brief Sets the description of the executing program. Displayed in the
     * help message.
     *
     * @param description Description string of the program.
     * @return ArgumentParser&
     */
    ArgumentParser& description(const std::string& description)
    {
        m_description = description;
        return *this;
    }

    /**
     * @brief Sets the version of the executing program. Displayed in the help
     * message.
     *
     * @param version Version string of the program.
     * @return ArgumentParser&
     */
    ArgumentParser& version(const std::string& version)
    {
        m_version = version;
        return *this;
    }

private:
    std::string m_name;
    std::string m_description;
    std::string m_version;

    uint32_t m_curr_arg = 1;
    std::vector<std::string> m_argv;

    std::unordered_map<std::string, size_t> m_options_map;
    std::vector<std::unique_ptr<Option>> m_options;
    std::vector<size_t> m_option_order;

    /**
     * @brief Consumes the next argument of the argument list and returns it.
     * Increments the internal argument pointer.
     *
     * @return std::string
     */
    std::string consume()
    {
        ++m_curr_arg;
        if (m_curr_arg >= m_argv.size())
            throw std::runtime_error("No more arguments.");
        return m_argv[m_curr_arg];
    }

    /**
     * @brief Parses options of type <option>=<value>.
     * Returns the option and a value, if present.
     *
     */
    std::tuple<std::string, std::optional<std::string>>
    parseOptionWithEqualSign(const std::string& arg)
    {
        auto equal_sign_pos = arg.find('=');
        if (equal_sign_pos != std::string::npos)
        {
            auto option = arg.substr(0, equal_sign_pos);
            auto value = arg.substr(equal_sign_pos + 1);
            return {option, {value}};
        }

        return {arg, {}};
    }

    void parseArguments()
    {
        while (m_curr_arg < m_argv.size())
        {
            auto arg = m_argv[m_curr_arg];
            auto [option, value] = parseOptionWithEqualSign(arg);
            if (value)
            {
                // if we have a option of type <option>=<value> with a value
                // store the value as if the arguments were <option> <value>
                m_argv.insert(m_argv.begin() + m_curr_arg + 1, value.value());
            }

            if (m_options_map.find(option) != m_options_map.end())
            {
                auto idx = m_options_map.at(option);
                auto& option = m_options.at(idx);

                if (option->flag)
                {
                    option->setValue({});
                }
                else
                {
                    // get the next argument and use it as value
                    std::string value = consume();

                    // check if the value is a option, thus the previous
                    // option with arguments was not satisified.
                    if (m_options_map.find(value) != m_options_map.end())
                    {
                        std::stringstream ss;
                        ss << "Expected argument after '" << option->name()
                           << "', but none given.";
                        throw ArgumentParserException(ss.str());
                    }

                    // pass the value to the option
                    option->setValue(value);
                }

                m_option_order.push_back(idx);
            }

            ++m_curr_arg;
        }
    }

    void checkRequiredOptions()
    {
        for (const auto& option : m_options)
        {
            if (option->required && !option->set)
            {
                std::stringstream ss;
                ss << "Option '" << option->name() << "' is required.";
                throw ArgumentParserException(ss.str());
            }
        }
    }

    void invokeCallbacks()
    {
        for (const auto& option_idx : m_option_order)
        {
            auto& option = m_options[option_idx];
            option->invokeCallback();
        }
    }

    bool checkOverrulingOptions()
    {
        for (const auto& option : m_options)
        {
            if (option->set && option->overruling)
            {
                option->invokeCallback();
                return true;
            }
        }

        return false;
    }
};
} // namespace clapp