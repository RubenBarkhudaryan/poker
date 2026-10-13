#ifndef TEXASSOLVER_API_PROTOCOL_H
#define TEXASSOLVER_API_PROTOCOL_H

// Wire format shared by TexasSolverServer and RemoteBackend.
// One JSON object per line:
//   request: {"cmd": "...", "args": {...}}
//   reply:   {"ok": true, "result": ...}  or  {"ok": false, "error": "..."}

#include "include/api/solver_dtos.h"
#include "include/json.hpp"

#include <QByteArray>

using json = nlohmann::json;

NLOHMANN_JSON_SERIALIZE_ENUM(NodeType, {
    {NodeType::Action, "action"},
    {NodeType::Chance, "chance"},
    {NodeType::Terminal, "terminal"},
    {NodeType::Showdown, "showdown"},
})

NLOHMANN_JSON_SERIALIZE_ENUM(GameRoundDto, {
    {GameRoundDto::Preflop, "preflop"},
    {GameRoundDto::Flop, "flop"},
    {GameRoundDto::Turn, "turn"},
    {GameRoundDto::River, "river"},
})

void to_json(json& j, const StreetSettingDto& v);
void from_json(const json& j, StreetSettingDto& v);
void to_json(json& j, const TreeBuildSettingsDto& v);
void from_json(const json& j, TreeBuildSettingsDto& v);
void to_json(json& j, const SolveRequest& v);
void from_json(const json& j, SolveRequest& v);
void to_json(json& j, const TreeNodeDto& v);
void from_json(const json& j, TreeNodeDto& v);
void to_json(json& j, const ActionLabelDto& v);
void from_json(const json& j, ActionLabelDto& v);
void to_json(json& j, const StrategyGridDto& v);
void from_json(const json& j, StrategyGridDto& v);
void to_json(json& j, const ProgressEvent& v);
void from_json(const json& j, ProgressEvent& v);

QByteArray encodeMessage(const json& message);

#endif // TEXASSOLVER_API_PROTOCOL_H
