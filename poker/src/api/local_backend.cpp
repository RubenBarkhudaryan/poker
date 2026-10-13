#include "include/api/local_backend.h"

#include <QString>
#include <algorithm>
#include <cmath>

using std::runtime_error;
using std::shared_ptr;
using std::string;
using std::vector;

LocalBackend::LocalBackend(QObject* parent) : QObject(parent) {}

void LocalBackend::setProgressCallback(ProgressCallback callback) {
    progress_ = std::move(callback);
}

void LocalBackend::emitProgress(const string& message, int iter, float expl) const {
    if (!progress_)
        return;
    ProgressEvent event;
    event.message = message;
    event.iteration = iter;
    event.exploitability = expl;
    progress_(event);
}

void LocalBackend::clearNodeMap() const {
    std::lock_guard<std::mutex> lock(nodes_mutex_);
    nodes_.clear();
}

shared_ptr<GameTreeNode> LocalBackend::findNode(const string& id) const {
    std::lock_guard<std::mutex> lock(nodes_mutex_);
    auto it = nodes_.find(id);
    if (it == nodes_.end())
        return nullptr;
    return it->second.lock();
}

StreetSetting LocalBackend::toStreetSetting(const StreetSettingDto& dto) {
    return StreetSetting(dto.bet_sizes, dto.raise_sizes, dto.donk_sizes, dto.allin);
}

GameTreeBuildingSettings LocalBackend::toTreeSettings(const TreeBuildSettingsDto& dto) {
    return GameTreeBuildingSettings(toStreetSetting(dto.flop_ip), toStreetSetting(dto.turn_ip),
                                    toStreetSetting(dto.river_ip), toStreetSetting(dto.flop_oop),
                                    toStreetSetting(dto.turn_oop), toStreetSetting(dto.river_oop));
}

GameRoundDto LocalBackend::toRoundDto(GameTreeNode::GameRound round) {
    switch (round) {
        case GameTreeNode::GameRound::PREFLOP:
            return GameRoundDto::Preflop;
        case GameTreeNode::GameRound::FLOP:
            return GameRoundDto::Flop;
        case GameTreeNode::GameRound::TURN:
            return GameRoundDto::Turn;
        case GameTreeNode::GameRound::RIVER:
            return GameRoundDto::River;
        default:
            throw runtime_error("unknown GameRound");
    }
}

NodeType LocalBackend::toNodeType(GameTreeNode::GameTreeNodeType type) {
    switch (type) {
        case GameTreeNode::GameTreeNodeType::ACTION:
            return NodeType::Action;
        case GameTreeNode::GameTreeNodeType::CHANCE:
            return NodeType::Chance;
        case GameTreeNode::GameTreeNodeType::TERMINAL:
            return NodeType::Terminal;
        case GameTreeNode::GameTreeNodeType::SHOWDOWN:
            return NodeType::Showdown;
        default:
            throw runtime_error("unknown GameTreeNodeType");
    }
}

string LocalBackend::actionLabel(GameTreeNode::PokerActions action, float amount) {
    switch (action) {
        case GameTreeNode::PokerActions::CHECK:
            return "CHECK";
        case GameTreeNode::PokerActions::BET:
            return "BET " + std::to_string(amount);
        case GameTreeNode::PokerActions::RAISE:
            return "RAISE " + std::to_string(amount);
        case GameTreeNode::PokerActions::FOLD:
            return "FOLD";
        case GameTreeNode::PokerActions::CALL:
            return "CALL";
        case GameTreeNode::PokerActions::BEGIN:
            return "BEGIN";
        case GameTreeNode::PokerActions::ROUNDBEGIN:
            return "ROUNDBEGIN";
        default:
            throw runtime_error("unknown PokerActions");
    }
}

string LocalBackend::nodeLabel(const shared_ptr<GameTreeNode>& node) {
    shared_ptr<GameTreeNode> parent = node->getParent();
    if (parent == nullptr) {
        switch (node->getRound()) {
            case GameTreeNode::GameRound::FLOP:
                return "FLOP begin";
            case GameTreeNode::GameRound::TURN:
                return "TURN begin";
            case GameTreeNode::GameRound::RIVER:
                return "RIVER begin";
            case GameTreeNode::GameRound::PREFLOP:
                return "PREFLOP begin";
            default:
                return "begin";
        }
    }

    if (parent->getType() == GameTreeNode::GameTreeNodeType::ACTION) {
        auto parent_action = std::dynamic_pointer_cast<ActionNode>(parent);
        vector<GameActions>& actions = parent_action->getActions();
        vector<shared_ptr<GameTreeNode>>& children = parent_action->getChildrens();
        for (std::size_t i = 0; i < children.size(); i++) {
            if (children[i] == node) {
                string seat = parent_action->getPlayer() == 0 ? "IP " : "OOP ";
                return seat + actionLabel(actions[i].getAction(), static_cast<float>(actions[i].getAmount()));
            }
        }
    }

    if (parent->getType() == GameTreeNode::GameTreeNodeType::CHANCE) {
        auto chance = std::dynamic_pointer_cast<ChanceNode>(parent);
        if (chance->getRound() == GameTreeNode::GameRound::FLOP)
            return "DEAL FLOP CARD";
        if (chance->getRound() == GameTreeNode::GameRound::TURN)
            return "DEAL TURN CARD";
        if (chance->getRound() == GameTreeNode::GameRound::RIVER)
            return "DEAL RIVER CARD";
    }

    return "NodeError";
}

void LocalBackend::registerNode(const string& id, const shared_ptr<GameTreeNode>& node) const {
    std::lock_guard<std::mutex> lock(nodes_mutex_);
    nodes_[id] = node;
}

TreeNodeDto LocalBackend::toDto(const shared_ptr<GameTreeNode>& node, const string& id) const {
    TreeNodeDto dto;
    dto.id = id;
    dto.label = nodeLabel(node);
    dto.node_type = toNodeType(node->getType());
    dto.round = toRoundDto(node->getRound());
    dto.pot = node->getPot();
    dto.player = -1;
    dto.has_children = false;

    if (node->getType() == GameTreeNode::GameTreeNodeType::ACTION) {
        auto action = std::dynamic_pointer_cast<ActionNode>(node);
        dto.player = action->getPlayer();
        dto.has_children = !action->getChildrens().empty();
    } else if (node->getType() == GameTreeNode::GameTreeNodeType::CHANCE) {
        auto chance = std::dynamic_pointer_cast<ChanceNode>(node);
        dto.has_children = chance->getChildren() != nullptr;
    }

    registerNode(id, node);
    return dto;
}

void LocalBackend::load() {
    clearNodeMap();

    const string suits = "c,d,h,s";
    const string resource_dir = ":/resources";

    emitProgress("Loading holdem comparing file");
    solver_ = PokerSolver("2,3,4,5,6,7,8,9,T,J,Q,K,A", suits,
                          resource_dir + "/compairer/card5_dic_sorted.txt", 2598961,
                          resource_dir + "/compairer/card5_dic_zipped.bin");

    emitProgress("Loading finished. Good to go.");
}

void LocalBackend::buildTree(const SolveRequest& request) {
    last_request_ = request;
    clearNodeMap();

    emitProgress("building tree..");
    GameTreeBuildingSettings settings = toTreeSettings(request.tree_settings);
    solver_.build_game_tree(request.oop_commit, request.ip_commit, request.current_round,
                            request.raise_limit, request.small_blind, request.big_blind, request.stack,
                            settings, request.allin_threshold);
    emitProgress("build tree finished");
}

long long LocalBackend::estimateMemory(const SolveRequest& request) {
    // Estimates against the tree of the current session, so last_request_ stays untouched.
    emitProgress("Estimating tree memory..");
    return solver_.estimate_tree_memory(QString::fromStdString(request.range_ip),
                                        QString::fromStdString(request.range_oop),
                                        QString::fromStdString(request.board));
}

void LocalBackend::solve(const SolveRequest& request) {
    last_request_ = request;

    emitProgress("Start Solving..");
    solver_.train(request.range_ip, request.range_oop, request.board, "", request.max_iteration,
                  request.print_interval, "discounted_cfr", -1, request.accuracy,
                  request.use_isomorphism != 0, request.use_halffloats, request.thread_number);
    emitProgress("Solving done.");
}

void LocalBackend::stop() {
    emitProgress("Trying to stop solver.");
    solver_.stop();
}

void LocalBackend::saveStrategy(const string& path, int dump_rounds) {
    emitProgress("Saving json file..");
    emitProgress("Dump round: " + std::to_string(dump_rounds));
    if (dump_rounds == 3) {
        emitProgress("This could be slow, or even blow your RAM, dump to river is not well optimized :(");
    }
    solver_.dump_strategy(QString::fromStdString(path), dump_rounds);
    emitProgress("Saving done.");
}

vector<string> LocalBackend::getRanks() const {
    return solver_.get_deck()->getRanks();
}

vector<string> LocalBackend::getDeckCards() const {
    vector<string> out;
    for (const Card& card : solver_.get_deck()->getCards())
        out.push_back(card.getCard());
    return out;
}

string LocalBackend::getBoard() const {
    return last_request_.board;
}

float LocalBackend::getStack() const {
    return last_request_.stack;
}

bool LocalBackend::hasSolver() const {
    return solver_.get_solver() != nullptr;
}

bool LocalBackend::hasGameTree() const {
    return solver_.get_game_tree() != nullptr &&
           solver_.get_game_tree()->getRoot() != nullptr;
}

TreeNodeDto LocalBackend::getTreeRoot() const {
    if (!hasGameTree())
        throw runtime_error("no game tree loaded");
    shared_ptr<GameTreeNode> root = solver_.get_game_tree()->getRoot();
    return toDto(root, "0");
}

const SolveRequest& LocalBackend::getLastRequest() const {
    return last_request_;
}

vector<TreeNodeDto> LocalBackend::getTreeChildren(const string& node_id) const {
    vector<TreeNodeDto> children;
    shared_ptr<GameTreeNode> node = findNode(node_id);
    if (node == nullptr)
        return children;

    if (node->getType() == GameTreeNode::GameTreeNodeType::ACTION) {
        auto action = std::dynamic_pointer_cast<ActionNode>(node);
        const vector<shared_ptr<GameTreeNode>>& kids = action->getChildrens();
        for (std::size_t i = 0; i < kids.size(); i++) {
            string child_id = node_id + "/" + std::to_string(i);
            children.push_back(toDto(kids[i], child_id));
        }
    } else if (node->getType() == GameTreeNode::GameTreeNodeType::CHANCE) {
        auto chance = std::dynamic_pointer_cast<ChanceNode>(node);
        shared_ptr<GameTreeNode> child = chance->getChildren();
        if (child != nullptr) {
            string child_id = node_id + "/0";
            children.push_back(toDto(child, child_id));
        }
    }
    return children;
}

StrategyGridDto LocalBackend::getStrategyAt(const string& node_id, int turn_card_int,
                                            int river_card_int) const {
    StrategyGridDto dto;
    if (!hasSolver() || !hasGameTree())
        return dto;

    shared_ptr<GameTreeNode> node = findNode(node_id);
    if (node == nullptr)
        return dto;

    PokerSolver* solver = const_cast<PokerSolver*>(&solver_);
    shared_ptr<Solver> cfr = solver->get_solver();
    if (cfr == nullptr)
        return dto;

    GameTreeNode::GameRound root_round = solver->get_game_tree()->getRoot()->getRound();
    dto.ranks = solver->get_deck()->getRanks();
    dto.root_round = toRoundDto(root_round);
    dto.node_round = toRoundDto(node->getRound());

    dto.card_int_to_str.assign(52, "");
    for (const Card& card : solver->get_deck()->getCards()) {
        int card_int = card.getCardInt();
        if (card_int >= 0 && card_int < 52)
            dto.card_int_to_str[card_int] = card.getCard();
    }

    if (node->getType() == GameTreeNode::GameTreeNodeType::ACTION) {
        auto action_node = std::dynamic_pointer_cast<ActionNode>(node);
        dto.current_player = action_node->getPlayer();

        for (GameActions& action : action_node->getActions()) {
            ActionLabelDto label;
            label.label = actionLabel(action.getAction(), static_cast<float>(action.getAmount()));
            label.amount = action.getAmount() < 0 ? 0.0f : static_cast<float>(action.getAmount());
            dto.actions.push_back(label);
        }

        vector<Card> deal_cards;
        GameTreeNode::GameRound current_round = action_node->getRound();
        if (root_round == GameTreeNode::GameRound::FLOP) {
            if (current_round == GameTreeNode::GameRound::TURN && turn_card_int >= 0)
                deal_cards.emplace_back(Card::intCard2Str(turn_card_int));
            if (current_round == GameTreeNode::GameRound::RIVER) {
                if (turn_card_int >= 0)
                    deal_cards.emplace_back(Card::intCard2Str(turn_card_int));
                if (river_card_int >= 0)
                    deal_cards.emplace_back(Card::intCard2Str(river_card_int));
            }
        } else if (root_round == GameTreeNode::GameRound::TURN) {
            if (current_round == GameTreeNode::GameRound::RIVER && river_card_int >= 0)
                deal_cards.emplace_back(Card::intCard2Str(river_card_int));
        }

        dto.strategy = cfr->get_strategy(action_node, deal_cards);
        dto.evs = cfr->get_evs(action_node, deal_cards);
    }

    dto.p1_range.assign(52, vector<float>(52, 0.0f));
    dto.p2_range.assign(52, vector<float>(52, 0.0f));
    for (const PrivateCards& one_private : solver->player1Range) {
        dto.p1_range[one_private.card1][one_private.card2] = one_private.weight;
        dto.p1_combos.emplace_back(one_private.card1, one_private.card2);
    }
    for (const PrivateCards& one_private : solver->player2Range) {
        dto.p2_range[one_private.card1][one_private.card2] = one_private.weight;
        dto.p2_combos.emplace_back(one_private.card1, one_private.card2);
    }

    // Condition ranges along the path (same idea as TableStrategyModel::updateStrategyData)
    shared_ptr<GameTreeNode> iter_node = node->getParent();
    shared_ptr<GameTreeNode> last_node = node;
    while (iter_node != nullptr) {
        if (iter_node->getType() == GameTreeNode::GameTreeNodeType::ACTION) {
            auto iter_action = std::dynamic_pointer_cast<ActionNode>(iter_node);
            vector<Card> path_deal_cards;
            if (root_round == GameTreeNode::GameRound::FLOP) {
                if (iter_action->getRound() == GameTreeNode::GameRound::TURN && turn_card_int >= 0)
                    path_deal_cards.emplace_back(Card::intCard2Str(turn_card_int));
                if (iter_action->getRound() == GameTreeNode::GameRound::RIVER) {
                    if (turn_card_int >= 0)
                        path_deal_cards.emplace_back(Card::intCard2Str(turn_card_int));
                    if (river_card_int >= 0)
                        path_deal_cards.emplace_back(Card::intCard2Str(river_card_int));
                }
            } else if (root_round == GameTreeNode::GameRound::TURN) {
                if (iter_action->getRound() == GameTreeNode::GameRound::RIVER && river_card_int >= 0)
                    path_deal_cards.emplace_back(Card::intCard2Str(river_card_int));
            }

            vector<vector<vector<float>>> path_strategy = cfr->get_strategy(iter_action, path_deal_cards);
            vector<shared_ptr<GameTreeNode>>& kids = iter_action->getChildrens();
            int child_chosen = -1;
            for (std::size_t i = 0; i < kids.size(); i++) {
                if (kids[i] == last_node) {
                    child_chosen = static_cast<int>(i);
                    break;
                }
            }
            if (child_chosen >= 0) {
                for (int i = 0; i < 52; i++) {
                    for (int j = 0; j < 52; j++) {
                        if (path_strategy[i][j].empty())
                            continue;
                        if (iter_action->getPlayer() == 0)
                            dto.p1_range[i][j] *= path_strategy[i][j][child_chosen];
                        else
                            dto.p2_range[i][j] *= path_strategy[i][j][child_chosen];
                    }
                }
            }
        }
        last_node = iter_node;
        iter_node = iter_node->getParent();
    }

    return dto;
}
