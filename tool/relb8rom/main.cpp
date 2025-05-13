#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <string>
#include <cstring>
#include <cstdint>
#include <algorithm>
#include "argparse.h"

const int addr_signature = 32;
const int addr_xreso = 40;
const int addr_yreso = 42;

void write_little_endian(std::ofstream& file, uint32_t value, std::streampos pos) {
    file.seekp(pos);
    file.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

void write_little_endian(std::ofstream& file, uint16_t value, std::streampos pos) {
    file.seekp(pos);
    file.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

int main(int argc, char* argv[]) {
    try {
        ArgumentParser parser("relb8rom");
        parser.add_argument("-i", "specify the raw ROM output by the gcc linker", true);
        parser.add_argument("-r", "specify the raw ROM output by genb8rom", true);
        parser.add_argument("-o", "B8 format to be released", true);
        parser.add_argument("-v", "increase output verbosity", false);
        parser.add_argument("-x", "x resolution", false);
        parser.add_argument("-y", "y resolution", false);
        parser.parse_args(argc, argv);
        std::string input_b8rom = parser.get("-i");
        std::string romfs = parser.get("-r");
        std::string output_b8 = parser.get("-o");
        int xreso = parser.get("-x").empty() ? 128 : std::stoi(parser.get("-x"));
        int yreso = parser.get("-y").empty() ? 240 : std::stoi(parser.get("-y"));

        if (xreso > 1920) {
            throw std::runtime_error("The given parameter value " + std::to_string(xreso) + " for xreso is invalid.");
        }

        if (yreso > 1920) {
            throw std::runtime_error("The given parameter value " + std::to_string(yreso) + " for yreso is invalid.");
        }

        std::ifstream fb8rom(input_b8rom, std::ios::binary);
        if (!fb8rom) {
            throw std::runtime_error("Cannot open input file: " + input_b8rom);
        }

        fb8rom.seekg(addr_signature);
        uint32_t nop;
        fb8rom.read(reinterpret_cast<char*>(&nop), sizeof(nop));
        if (nop != 0xe1a00000) {
            throw std::runtime_error(input_b8rom + " is invalid");
        }

        fb8rom.seekg(0, std::ios::end);
        std::vector<char> b8rom(fb8rom.tellg());
        fb8rom.seekg(0, std::ios::beg);
        fb8rom.read(b8rom.data(), b8rom.size());
        fb8rom.close();

        std::ifstream fromfs(romfs, std::ios::binary);
        if (!fromfs) {
            throw std::runtime_error("Cannot open ROMFS file: " + romfs);
        }

        fromfs.seekg(0, std::ios::end);
        std::streamsize romfs_size = fromfs.tellg();
        fromfs.seekg(0, std::ios::beg);
        b8rom.resize(b8rom.size() + romfs_size);
        fromfs.read(b8rom.data() + (b8rom.size() - romfs_size), romfs_size);
        fromfs.close();

        std::ofstream output(output_b8, std::ios::binary);
        output.write(b8rom.data(), b8rom.size());

        write_little_endian(output, static_cast<uint32_t>(b8rom.size() - romfs_size), addr_signature);
        write_little_endian(output, static_cast<uint16_t>(xreso), addr_xreso);
        write_little_endian(output, static_cast<uint16_t>(yreso), addr_yreso);

        output.close();

    } catch (const std::exception& e) {
        std::cerr << "\033[1;31mdecode_b8.cpp Exception: " << e.what() << "\033[0m" << std::endl;
        return -1;
    }

    return 0;
}
