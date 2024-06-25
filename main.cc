#include <bit>
#include <random>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <exception>

#include <nlohmann/json.hpp>
#include <argparse/argparse.hpp>

using json = nlohmann::json;

using u32 = unsigned int;
using u64 = unsigned long long;

void formatError(std::string fn) {
    std::cerr << "wrong format in \'" + fn + "\'." << std::endl;
    exit(1);
}

inline void op(u64 &a, u64 b) {
    a = (a >> 3) | (a << 61);
    a ^= b;
    a ^= 1ULL << std::popcount(a);
}

int main(int argc, char *argv[]) {
    argparse::ArgumentParser program("frandom", "1.0");

    auto &group = program.add_mutually_exclusive_group();

    group.add_argument("-g", "--gen")
      .nargs(1)
      .default_value(std::string("a.json"))
      .help("generate a data file.");

    group.add_argument("-c", "--calc")
      .nargs(2)
      .default_value(std::vector<std::string>({"a.json", "b.json"}))
      .help("calculate result of two data files.");

    program.add_argument("-l", "--length")
      .nargs(1)
      .scan<'i', int>()
      .default_value(29);
    
    try {
        program.parse_args(argc, argv);
    }
    catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }

    if (program.is_used("-g")) {
        auto fn = program.get<std::string>("-g");
        
        std::mt19937_64 rng(std::random_device{}());

        json o;
        for (int i = 0; i < program.get<int>("-l"); i++) o.push_back(rng());

        std::ofstream F(fn);
        F << o;
    }

    if (program.is_used("-c")) {
        auto fn = program.get<std::vector<std::string>>("-c");

        if (fn.size() == 1) {
            std::cerr << "Two arguments required." << std::endl;
            std::cerr << program;
            std::exit(1);
        }

        std::ifstream F1(fn[0]), F2(fn[1]);
        json data1 = json::parse(F1), data2 = json::parse(F2);

        std::vector<u64> v1, v2;
        if (!data1.is_array()) formatError(fn[0]);
        for (auto x : data1) {
            if (x.is_number_unsigned()) v1.push_back(x);
            else formatError(fn[0]);
        }
        if (!data2.is_array()) formatError(fn[1]);
        for (auto x : data2) {
            if (x.is_number_unsigned()) v2.push_back(x);
            else formatError(fn[1]);
        }
        if (v1.size() != v2.size()) {
            std::cerr << "length are diffrent." << std::endl;
            exit(1);
        }
        if (v1.size() > 30) {
            std::cerr << "too long to calculate." << std::endl;
            exit(1);
        }

        int sz = v1.size();

        u64 rs = 0;
        for (int s = 0; s < (1 << sz); s++) {
            for (int i = 0; i < sz; i++) {
                if ((s >> i) & 1) op(rs, v1[i]);
                else op(rs, v2[i]);
            }
        }

        std::cout << rs;
    }
}