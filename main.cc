#include <bit>
#include <random>
#include <string>
#include <vector>
#include <fstream>
#include <iterator>
#include <iostream>
#include <algorithm>

#include <argparse/argparse.hpp>

using u64 = uint64_t;

std::vector<u64> ifVector(std::string fn) {
    std::ifstream in(fn, std::ifstream::binary);
    if (!in) {
        std::cerr << "cannot open file \'" + fn + "\'.";
        exit(1);
    }

    std::vector<char> rs8;
    std::copy(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>(), std::back_inserter(rs8));

    if (rs8.size() % 8 != 0) rs8.resize((rs8.size() + 7) / 8 * 8);
    std::vector<u64> rs64((u64 *)rs8.data(), (u64 *)(rs8.data() + rs8.size()));

    in.close();
    return rs64;
}

void ofVector(std::string fn, const std::vector<u64> &v64) {
    std::vector<char> v8((char *)v64.data(), (char *)(v64.data() + v64.size()));

    std::ofstream o(fn, std::ofstream::binary);
    std::copy(v8.begin(), v8.end(), std::ostreambuf_iterator<char>(o));

    o.flush();
    o.close();
}

int main(int argc, char *argv[]) {
    argparse::ArgumentParser program("frandom", "v1.0.0");

    auto &group = program.add_mutually_exclusive_group();

    group.add_argument("-g", "--gen")
      .nargs(1)
      .default_value(std::string("a.bin"))
      .help("generate a data file.");

    group.add_argument("-c", "--calc")
      .nargs(2)
      .default_value(std::vector<std::string>({"a.bin", "b.bin"}))
      .help("calculate result of two data files.");

    program.add_argument("-l", "--length")
      .nargs(1)
      .scan<'i', int>()
      .default_value(29);
    
    program.parse_args(argc, argv);

    if (program.is_used("-g")) {
        auto fn = program.get<std::string>("-g");
        
        std::mt19937_64 rng(std::random_device{}());

        std::vector<u64> o;
        for (int i = 0; i < program.get<int>("-l"); i++) o.push_back(rng());

        ofVector(fn, o);
    }

    if (program.is_used("-c")) {
        auto fn = program.get<std::vector<std::string>>("-c");

        if (fn.size() == 1) {
            std::cerr << "Two arguments required." << std::endl;
            std::cerr << program;
            std::exit(1);
        }

        std::ifstream F1(fn[0], std::ios::binary), F2(fn[1], std::ios::binary);

        std::vector<u64> v1 = ifVector(fn[0]);
        std::vector<u64> v2 = ifVector(fn[1]);
        if (v1.size() != v2.size()) {
            std::cerr << "length are diffrent." << std::endl;
            exit(1);
        }
        if (v1.size() > 35) {
            std::cerr << "too long to calculate." << std::endl;
            exit(1);
        }

        int sz = v1.size();
        std::vector<u64> vmg;
        for (int i = 0; i < sz; i++) vmg.push_back(v2[i]), vmg.push_back(v1[i]);

        u64 rs = 0;
        for (u64 s = 0; s < (1ULL << sz); s++) {
            u64 *p = vmg.data(), t = s;
            for (int i = 0; i < sz; i++) {
                rs += *(p + 2 - (t & 1));
                rs = rs * rs;
                rs += *(p + (t & 1));
                rs = std::rotr(rs, std::popcount(rs));
                p += 2;
                t >>= 1;
            }
        }

        std::cout << rs << std::endl;
    }
}