#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <iostream>
#include <algorithm>

class ArgumentParser {
public:
    ArgumentParser(const std::string& description = "") : description(description) {
        add_argument("-h", "show this help message and exit", false);
    }

    void add_argument(const std::string& name, const std::string& help = "", bool required = false) {
        args[name] = {help, required, ""};
    }

    void parse_args(int argc, char* argv[]) {
        if (argc == 1) {
            print_help();
            std::exit(0);
        }
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "-h") {
                print_help();
                std::exit(0);
            }
            if (args.find(arg) != args.end()) {
                if (i + 1 < argc && args.find(argv[i + 1]) == args.end()) {
                    args[arg].value = argv[++i];
                } else if (args[arg].required) {
                    throw std::runtime_error("Argument " + arg + " requires a value");
                }
            } else {
                throw std::runtime_error("Unknown argument: " + arg);
            }
        }
        for (const auto& [key, val] : args) {
            if (val.required && val.value.empty()) {
                throw std::runtime_error("Required argument " + key + " is missing");
            }
        }
    }

    std::string get(const std::string& name) const {
        if (args.find(name) != args.end()) {
            return args.at(name).value;
        }
        throw std::runtime_error("Argument " + name + " not found");
    }

    void print_help() const {
        std::cout << "usage:\n";
        // Create a vector of keys and sort it
        std::vector<std::string> keys;
        for (const auto& [key, _] : args) {
            keys.push_back(key);
        }
        std::sort(keys.begin(), keys.end());
        // Print sorted arguments
        for (const auto& key : keys) {
            const auto& val = args.at(key);
            std::cout << "  " << key << " " << val.help << (val.required ? " (required)" : "") << std::endl;
        }
    }

private:
    struct ArgInfo {
        std::string help;
        bool required;
        std::string value;
    };

    std::unordered_map<std::string, ArgInfo> args;
    std::string description;
};
