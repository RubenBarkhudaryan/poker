#include "include/api/protocol.h"

#include <limits>

#define TO(field) j[#field] = v.field
#define FROM(field)            \
    if (j.contains(#field))    \
    j.at(#field).get_to(v.field)

namespace
{
    // nlohmann writes NaN/inf as null, and get<float>() on null throws.
    float readFloat(const json& x)
    {
        return x.is_null() ? std::numeric_limits<float>::quiet_NaN() : x.get<float>();
    }

    std::vector<std::vector<float>> readGrid2(const json& j)
    {
        std::vector<std::vector<float>> out(j.size());
        for (std::size_t i = 0; i < j.size(); ++i) {
            out[i].reserve(j[i].size());
            for (const json& x : j[i])
                out[i].push_back(readFloat(x));
        }
        return out;
    }

    std::vector<std::vector<std::vector<float>>> readGrid3(const json& j)
    {
        std::vector<std::vector<std::vector<float>>> out(j.size());
        for (std::size_t i = 0; i < j.size(); ++i)
            out[i] = readGrid2(j[i]);
        return out;
    }
}

void to_json(json& j, const StreetSettingDto& v)
{
    TO(bet_sizes);
    TO(raise_sizes);
    TO(donk_sizes);
    TO(allin);
}

void from_json(const json& j, StreetSettingDto& v)
{
    FROM(bet_sizes);
    FROM(raise_sizes);
    FROM(donk_sizes);
    FROM(allin);
}

void to_json(json& j, const TreeBuildSettingsDto& v)
{
    TO(flop_ip);
    TO(turn_ip);
    TO(river_ip);
    TO(flop_oop);
    TO(turn_oop);
    TO(river_oop);
}

void from_json(const json& j, TreeBuildSettingsDto& v)
{
    FROM(flop_ip);
    FROM(turn_ip);
    FROM(river_ip);
    FROM(flop_oop);
    FROM(turn_oop);
    FROM(river_oop);
}

void to_json(json& j, const SolveRequest& v)
{
    TO(oop_commit);
    TO(ip_commit);
    TO(current_round);
    TO(raise_limit);
    TO(thread_number);
    TO(small_blind);
    TO(big_blind);
    TO(stack);
    TO(allin_threshold);
    TO(range_ip);
    TO(range_oop);
    TO(board);
    TO(accuracy);
    TO(max_iteration);
    TO(use_isomorphism);
    TO(use_halffloats);
    TO(print_interval);
    TO(dump_rounds);
    TO(tree_settings);
}

void from_json(const json& j, SolveRequest& v)
{
    FROM(oop_commit);
    FROM(ip_commit);
    FROM(current_round);
    FROM(raise_limit);
    FROM(thread_number);
    FROM(small_blind);
    FROM(big_blind);
    FROM(stack);
    FROM(allin_threshold);
    FROM(range_ip);
    FROM(range_oop);
    FROM(board);
    FROM(accuracy);
    FROM(max_iteration);
    FROM(use_isomorphism);
    FROM(use_halffloats);
    FROM(print_interval);
    FROM(dump_rounds);
    FROM(tree_settings);
}

void to_json(json& j, const TreeNodeDto& v)
{
    TO(id);
    TO(label);
    TO(node_type);
    TO(round);
    TO(player);
    TO(pot);
    TO(has_children);
}

void from_json(const json& j, TreeNodeDto& v)
{
    FROM(id);
    FROM(label);
    FROM(node_type);
    FROM(round);
    FROM(player);
    FROM(pot);
    FROM(has_children);
}

void to_json(json& j, const ActionLabelDto& v)
{
    TO(label);
    TO(amount);
}

void from_json(const json& j, ActionLabelDto& v)
{
    FROM(label);
    FROM(amount);
}

void to_json(json& j, const StrategyGridDto& v)
{
    TO(ranks);
    TO(actions);
    TO(current_player);
    TO(root_round);
    TO(node_round);
    TO(strategy);
    TO(evs);
    TO(p1_range);
    TO(p2_range);
    TO(p1_combos);
    TO(p2_combos);
    TO(card_int_to_str);
}

void from_json(const json& j, StrategyGridDto& v)
{
    FROM(ranks);
    FROM(actions);
    FROM(current_player);
    FROM(root_round);
    FROM(node_round);
    FROM(p1_combos);
    FROM(p2_combos);
    FROM(card_int_to_str);
    if (j.contains("strategy"))
        v.strategy = readGrid3(j.at("strategy"));
    if (j.contains("evs"))
        v.evs = readGrid3(j.at("evs"));
    if (j.contains("p1_range"))
        v.p1_range = readGrid2(j.at("p1_range"));
    if (j.contains("p2_range"))
        v.p2_range = readGrid2(j.at("p2_range"));
}

void to_json(json& j, const ProgressEvent& v)
{
    TO(iteration);
    TO(exploitability);
    TO(message);
}

void from_json(const json& j, ProgressEvent& v)
{
    FROM(iteration);
    FROM(exploitability);
    FROM(message);
}

QByteArray encodeMessage(const json& message)
{
    // dump() escapes control characters, so '\n' only ever appears as the terminator.
    return QByteArray::fromStdString(message.dump()) + '\n';
}
