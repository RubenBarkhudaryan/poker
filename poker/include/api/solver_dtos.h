#ifndef TEXASSOLVER_API_SOLVER_DTOS_H
#define TEXASSOLVER_API_SOLVER_DTOS_H

#include <string>
#include <utility>
#include <vector>

enum class NodeType
{
    Action,
    Chance,
    Terminal,
    Showdown
};

enum class GameRoundDto
{
    Preflop,
    Flop,
    Turn,
    River
};

struct StreetSettingDto
{
    std::vector<float> bet_sizes;
    std::vector<float> raise_sizes;
    std::vector<float> donk_sizes;
    bool allin = false;
};

struct TreeBuildSettingsDto
{
    StreetSettingDto flop_ip;
    StreetSettingDto turn_ip;
    StreetSettingDto river_ip;
    StreetSettingDto flop_oop;
    StreetSettingDto turn_oop;
    StreetSettingDto river_oop;
};

struct SolveRequest
{
    float oop_commit = 5.0f;
    float ip_commit = 5.0f;

    int current_round = 1;
    int raise_limit = 4;
    int thread_number = 1;
    float small_blind = 0.5f;
    float big_blind = 1.0f;
    float stack = 25.0f;
    float allin_threshold = 0.67f;

    std::string range_ip;
    std::string range_oop;
    std::string board;

    float accuracy = 0.0f;
    int max_iteration = 100;
    int use_isomorphism = 1;
    int use_halffloats = 0;
    int print_interval = 10;
    int dump_rounds = 2;

    TreeBuildSettingsDto tree_settings;
};

struct TreeNodeDto
{
    std::string id;
    std::string label;
    NodeType node_type = NodeType::Action;
    GameRoundDto round = GameRoundDto::Flop;
    int player = -1;
    double pot = 0.0;
    bool has_children = false;
};

struct ActionLabelDto
{
    std::string label;
    float amount = 0.0f;
};

struct StrategyGridDto
{
    std::vector<std::string> ranks;
    std::vector<ActionLabelDto> actions;

    int current_player = 0;
    GameRoundDto root_round = GameRoundDto::Flop;
    GameRoundDto node_round = GameRoundDto::Flop;

    std::vector<std::vector<std::vector<float>>> strategy;
    std::vector<std::vector<std::vector<float>>> evs;

    std::vector<std::vector<float>> p1_range;
    std::vector<std::vector<float>> p2_range;

    // Every (card1, card2) in each player's starting range, including combos
    // whose conditioned weight in p1_range / p2_range has dropped to zero.
    std::vector<std::pair<int, int>> p1_combos;
    std::vector<std::pair<int, int>> p2_combos;

    std::vector<std::string> card_int_to_str;
};

struct ProgressEvent
{
    int iteration = 0;
    float exploitability = 0.0f;
    std::string message;
};

#endif // TEXASSOLVER_API_SOLVER_DTOS_H
