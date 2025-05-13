#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <stdexcept>
#include <cstring>
#include <cstdint>
#include <regex>
#include "argparse.h"

using namespace std;
namespace fs = std::filesystem;

const uint16_t fat_section_start = 16;
const uint8_t one_fat_bytesize = 48;
const uint32_t max_len_of_fname = one_fat_bytesize - 2 * 4;

class OneFile {
public:
    string fname;
    uint32_t len;
    uint32_t offset;

    OneFile(const string& filename, uint32_t length, uint32_t offset)
        : fname(fs::path(filename).filename().string()), len(length), offset(offset) {}
};

bool match_pattern(const string& pattern, const string& str) {
    string regex_pattern = std::regex_replace(pattern, std::regex("\\*"), ".*");
    return std::regex_match(str, std::regex(regex_pattern));
}

int main(int argc, char* argv[]) {
    ArgumentParser program("genromfs");

    program.add_argument("-i", "input path or pattern", true);
    program.add_argument("-o", "output bin file", true);
    program.add_argument("-v", "increase output verbosity", false);

    try {
        program.parse_args(argc, argv);
    } catch (const runtime_error& err) {
        cerr << err.what() << endl;
        program.print_help();
        return -1;
    }

    bool verbose = !program.get("-v").empty();
    string input_pattern = program.get("-i");
    string out_bin = program.get("-o");

    if (verbose) {
        cout << "input_pattern: " << input_pattern << endl;
        cout << "out_bin: " << out_bin << endl;
        cout << "verbose: " << verbose << endl;
    }

    string input_path = fs::path(input_pattern).parent_path().string();
    string file_pattern = fs::path(input_pattern).filename().string();

    if (input_path.empty()) {
        input_path = ".";
    }

    vector<OneFile> file_list;
    vector<uint8_t> packdata;

    for (const auto& entry : fs::directory_iterator(input_path)) {
        if (entry.is_regular_file() && match_pattern(file_pattern, entry.path().filename().string())) {
            string file = entry.path().string();

            if (fs::path(file).filename().string().length() >= max_len_of_fname) {
                throw runtime_error("file name " + file + " exceeds " + to_string(max_len_of_fname) + " bytes in length");
            }

            ifstream fr(file, ios::binary);
            if (!fr) {
                throw runtime_error("failed to open file: " + file);
            }

            vector<uint8_t> fdata((istreambuf_iterator<char>(fr)), istreambuf_iterator<char>());
            fr.close();

            OneFile onefile(file, fdata.size(), packdata.size());
            file_list.push_back(onefile);

            packdata.insert(packdata.end(), fdata.begin(), fdata.end());

            size_t npad = (packdata.size() + 4) - (packdata.size() & 3) - packdata.size();
            if (npad < 4) {
                packdata.insert(packdata.end(), npad, 0);
            }
        }
    }

    ofstream fat(out_bin, ios::binary);
    if (!fat) {
        throw runtime_error("failed to open output file: " + out_bin);
    }

    fat.write("BP8R", 4);
    uint16_t num_files = file_list.size();
    fat.write(reinterpret_cast<char*>(&num_files), 2);
    fat.write(reinterpret_cast<const char*>(&fat_section_start), 2);
    fat.write(reinterpret_cast<const char*>(&one_fat_bytesize), 1);
    vector<uint8_t> header_padding(fat_section_start - (4 + 2 + 2 + 1), 0);
    fat.write(reinterpret_cast<char*>(header_padding.data()), header_padding.size());

    for (const auto& of : file_list) {
      fat.write(reinterpret_cast<const char*>(&of.offset), 4);
      fat.write(reinterpret_cast<const char*>(&of.len), 4);
      vector<uint8_t> fname_bytes(of.fname.begin(), of.fname.end());
      vector<uint8_t> fname_padding(one_fat_bytesize - 2 * 4 - fname_bytes.size(), 0);
      fat.write(reinterpret_cast<char*>(fname_bytes.data()), fname_bytes.size());
      fat.write(reinterpret_cast<char*>(fname_padding.data()), fname_padding.size());
    }

    fat.write(reinterpret_cast<char*>(packdata.data()), packdata.size());
    fat.close();

    return 0;
}
