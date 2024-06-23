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

std::vector<u64> parseVars(const json &a, std::string fn) {
    if (a["vars"].is_array() && a["vars"].size() == 128) {
        std::vector<u64> rs;
        for (auto x : a["vars"]) {
            if (x.is_number_unsigned()) rs.push_back(x);
            else formatError(fn);
        }
        return rs;
    } else formatError(fn);
}

std::vector<std::vector<int>> parseFuncs(const json &a, std::string fn) {
    if (a["funcs"].is_array()) {
        std::vector<std::vector<int>> rs;
        int funcCnt = 0;
        for (auto x : a["funcs"]) {
            rs.emplace_back();
            if (x.is_array()) {
                for (auto y : x) {
                    if (y.is_number_unsigned() && y < 128 + funcCnt) rs.back().push_back(y);
                    else formatError(fn);
                }
            } else formatError(fn);
            funcCnt++;
        }
        return rs;
    } else formatError(fn);
}

void op(u64 &a, u64 b) {
    a = ~a ^ b;
    a ^= 1ULL << __builtin_popcountll(a);
}

u64 getResult(int id, const std::vector<std::vector<int>> &f, const std::vector<u64> &v, u64 &ans) {
    u64 opcount = 0;
    for (int nxt : f[id]) {
        if (nxt < 128) {
            op(ans, v[nxt]);
            opcount++;
        } else opcount += getResult(nxt - 128, f, v, ans);
    }
    op(ans, id);
    return ++opcount;
}

const u64 minimumTimeComplexity = 1000000000;

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
        o["vars"] = json::array();
        for (int i = 0; i < 128; i++) o["vars"].push_back(rng());

        o["funcs"] = json::array();
        std::vector<u64> timeComplexity;
        int funcCnt = 0;
        while (true) {
            o["funcs"].push_back(json::array());
            timeComplexity.push_back(1);
            std::uniform_int_distribution d(0, funcCnt - 1);
            while (rng() % 512 != 0 && timeComplexity.back() < minimumTimeComplexity) {
                if (funcCnt == 0 || rng() % 128 != 0) {
                    o["funcs"].back().push_back(rng() % 128);
                    timeComplexity.back()++;
                } else {
                    int nw = d(rng);
                    o["funcs"].back().push_back(128 + nw);
                    timeComplexity.back() += timeComplexity[nw];
                }
            }
            funcCnt++;
            if (timeComplexity.back() >= minimumTimeComplexity) break;
        }

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

        auto v1 = parseVars(data1, fn[0]), v2 = parseVars(data2, fn[1]);
        auto f1 = parseFuncs(data1, fn[0]), f2 = parseFuncs(data2, fn[1]);

        u64 rs = 0;
        if (getResult(f1.size() - 1, f1, v2, rs) < minimumTimeComplexity) std::cerr << "complexity is too low in \'" + fn[0] + "\'." << std::endl;
        if (getResult(f2.size() - 1, f2, v1, rs) < minimumTimeComplexity) std::cerr << "complexity is too low in \'" + fn[1] + "\'." << std::endl;
        std::cout << rs;
    }
}