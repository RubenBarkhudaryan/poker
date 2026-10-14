// Solves one spot with the OpenCL solver and the CPU solver and compares the resulting strategies.
//
// Usage: TexasSolverBench <resource_dir> <board> <iterations> [gpu|cpu|both] [half_floats]
//   board: comma separated cards; 3 cards solve from the flop, 4 from the turn, 5 on the river.

#include "include/runtime/PokerSolver.h"
#include <QtCore>
#include <chrono>
#include <cmath>

namespace {

const char* kRangeIp =
    "AA,KK,QQ,JJ,TT,99:0.75,88:0.75,77:0.5,66:0.25,55:0.25,AK,AQs,AQo:0.75,AJs,AJo:0.5,ATs:0.75,A6s:0.25,"
    "A5s:0.75,A4s:0.75,A3s:0.5,A2s:0.5,KQs,KQo:0.5,KJs,KTs:0.75,K5s:0.25,K4s:0.25,QJs:0.75,QTs:0.75,Q9s:0.5,"
    "JTs:0.75,J9s:0.75,J8s:0.75,T9s:0.75,T8s:0.75,T7s:0.75,98s:0.75,97s:0.75,96s:0.5,87s:0.75,86s:0.5,85s:0.5,"
    "76s:0.75,75s:0.5,65s:0.75,64s:0.5,54s:0.75,53s:0.5,43s:0.5";
const char* kRangeOop =
    "QQ:0.5,JJ:0.75,TT,99,88,77,66,55,44,33,22,AKo:0.25,AQs,AQo:0.75,AJs,AJo:0.75,ATs,ATo:0.75,A9s,A8s,A7s,"
    "A6s,A5s,A4s,A3s,A2s,KQ,KJ,KTs,KTo:0.5,K9s,K8s,K7s,K6s,K5s,K4s:0.5,K3s:0.5,K2s:0.5,QJ,QTs,Q9s,Q8s,Q7s,"
    "JTs,JTo:0.5,J9s,J8s,T9s,T8s,T7s,98s,97s,96s,87s,86s,76s,75s,65s,64s,54s,53s,43s";

struct Result {
    json strategy;
    double seconds;
};

Result solve(const string& resource_dir, const string& board, int iterations, bool gpu, int half_floats) {
    if (gpu)
        qunsetenv("TEXASSOLVER_DISABLE_OPENCL");
    else
        qputenv("TEXASSOLVER_DISABLE_OPENCL", "1");

    PokerSolver ps("2,3,4,5,6,7,8,9,T,J,Q,K,A", "c,d,h,s", resource_dir + "/compairer/card5_dic_sorted.txt", 2598961,
                   resource_dir + "/compairer/card5_dic_zipped.bin");
    StreetSetting street(vector<float>{50, 100}, vector<float>{60}, vector<float>{}, true);
    GameTreeBuildingSettings settings(street, street, street, street, street, street);
    int board_cards = count(board.begin(), board.end(), ',') + 1;
    ps.build_game_tree(5, 5, board_cards - 2, 3, 0.5, 1, 100, settings, 0.67);

    auto begin = chrono::steady_clock::now();
    ps.train(kRangeIp, kRangeOop, board, "", iterations, max(1, iterations / 4), "discounted_cfr", -1, 0, true,
             half_floats, 1);
    double seconds = chrono::duration<double>(chrono::steady_clock::now() - begin).count();
    return {ps.get_solver()->dumps(false, 2), seconds};
}

// Largest difference between two strategy dumps, over every action probability.
struct Diff {
    double max = 0, sum = 0;
    long n = 0, over = 0;
    string where;
};

void compare(const json& a, const json& b, const string& path, Diff& diff) {
    if (a.is_number() && b.is_number()) {
        double d = fabs(a.get<double>() - b.get<double>());
        if (d > diff.max) {
            diff.max = d;
            diff.where = path;
        }
        diff.sum += d;
        diff.n++;
        if (d > 1e-3)
            diff.over++;
    } else if (a.is_array() && b.is_array() && a.size() == b.size()) {
        for (std::size_t i = 0; i < a.size(); i++)
            compare(a[i], b[i], path + "[" + to_string(i) + "]", diff);
    } else if (a.is_object() && b.is_object()) {
        for (auto it = a.begin(); it != a.end(); ++it) {
            if (!b.contains(it.key()))
                throw runtime_error("strategy dumps differ in structure at " + path + "/" + it.key());
            compare(it.value(), b[it.key()], path + "/" + it.key(), diff);
        }
    }
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 4) {
        cerr << "usage: TexasSolverBench <resource_dir> <board> <iterations> [gpu|cpu|both] [half_floats]" << endl;
        return 2;
    }
    string resource_dir = argv[1];
    string board = argv[2];
    int iterations = atoi(argv[3]);
    string which = argc > 4 ? argv[4] : "both";
    int half_floats = argc > 5 ? atoi(argv[5]) : 0;

    Result gpu_result, cpu_result;
    if (which != "cpu") {
        gpu_result = solve(resource_dir, board, iterations, true, half_floats);
        cout << "OpenCL solve: " << gpu_result.seconds << " s" << endl;
    }
    if (which != "gpu") {
        cpu_result = solve(resource_dir, board, iterations, false, half_floats);
        cout << "CPU solve: " << cpu_result.seconds << " s" << endl;
    }
    if (which == "both") {
        Diff diff;
        compare(gpu_result.strategy, cpu_result.strategy, "", diff);
        cout << "Compared " << diff.n << " values: max diff " << diff.max << " at " << diff.where << ", mean diff "
             << (diff.n ? diff.sum / diff.n : 0) << ", " << diff.over << " values differ by more than 0.001"
             << endl;
    }
    return 0;
}
