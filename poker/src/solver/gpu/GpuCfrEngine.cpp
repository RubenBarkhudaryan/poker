#include "src/solver/gpu/GpuCfrEngine.h"
#include "cfr_kernels_cl.h"
#include "include/tools/utils.h"
#include <QObject>
#include <QtCore>
#include <algorithm>
#include <bitset>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <numeric>
#include <thread>

namespace {

const int kGroup2d = 64;

float halfToFloat(uint16_t h) {
    uint32_t sign = (uint32_t)(h & 0x8000) << 16;
    uint32_t exponent = (h >> 10) & 0x1f;
    uint32_t mantissa = h & 0x3ff;
    uint32_t bits;
    if (exponent == 0) {
        if (mantissa == 0) {
            bits = sign;
        } else {
            // Subnormal: normalise it.
            exponent = 113;
            while ((mantissa & 0x400) == 0) {
                mantissa <<= 1;
                exponent--;
            }
            bits = sign | (exponent << 23) | ((mantissa & 0x3ff) << 13);
        }
    } else if (exponent == 31) {
        bits = sign | 0x7f800000 | (mantissa << 13);
    } else {
        bits = sign | ((exponent + 112) << 23) | (mantissa << 13);
    }
    float value;
    memcpy(&value, &bits, sizeof(value));
    return value;
}

// Index of an unordered pair of suits, 0..5.
int suitPairIndex(int a, int b) {
    if (a > b)
        swap(a, b);
    static const int index[4][4] = {{-1, 0, 1, 2}, {0, -1, 3, 4}, {1, 3, -1, 5}, {2, 4, 5, -1}};
    return index[a][b];
}

template <typename T>
size_t bytesOf(const vector<T>& v) {
    return v.size() * sizeof(T);
}

} // namespace

bool GpuCfrEngine::disabledByEnvironment() {
    const char* value = getenv("TEXASSOLVER_DISABLE_OPENCL");
    return value != nullptr && string(value) != "" && string(value) != "0";
}

GpuCfrEngine::GpuCfrEngine(const shared_ptr<GameTree>& tree, const vector<vector<PrivateCards>>& ranges,
                           PrivateCardsManager& pcm, const shared_ptr<Compairer>& compairer, Deck& deck,
                           const int color_iso_offset[][4], int use_halffloats, uint64_t initial_board)
    : deck(deck), pcm(pcm), compairer(compairer), ranges(ranges), color_iso_offset(color_iso_offset),
      use_halffloats(use_halffloats), initial_board(initial_board) {
    card_num = deck.getCards().size();
    n_hands[0] = ranges[0].size();
    n_hands[1] = ranges[1].size();
    H = max(1, max(n_hands[0], n_hands[1]));

    cl = make_unique<OpenClContext>();

    allocate(0, 1);
    expand(tree->getRoot(), 0, 0, 0, initial_board);
    buildHandTables();
    buildBoardTables();
    checkMemory();
    buildKernels();
    upload();
}

GpuCfrEngine::~GpuCfrEngine() {
    for (cl_kernel kernel : {k_pull, k_terminal, k_showdown, k_chance, k_action, k_ev})
        if (kernel != nullptr)
            clReleaseKernel(kernel);
    if (program != nullptr)
        clReleaseProgram(program);
    for (cl_mem buffer : buffers)
        clReleaseMemObject(buffer);
}

string GpuCfrEngine::description() const {
    size_t instances = 0;
    for (const Level& level : levels)
        instances += level.n_inst;
    return tfm::format("%s, %s tree levels, %s node instances", cl->description(), levels.size(), instances);
}

int GpuCfrEngine::allocate(int depth, int count) {
    if ((int)levels.size() <= depth) {
        levels.resize(depth + 1);
        write_back.resize(depth + 1);
    }
    Level& level = levels[depth];
    int first = level.n_inst;
    level.n_inst += count;
    level.pulls.resize(level.n_inst);
    return first;
}

int GpuCfrEngine::boardId(uint64_t board) {
    auto it = board_ids.find(board);
    if (it != board_ids.end())
        return it->second;
    int id = boards.size();
    board_ids[board] = id;
    boards.push_back(board);
    return id;
}

// Mirrors PCfrSolver::cfr: the same deals are visited and the same trainables are trained.
void GpuCfrEngine::expand(const shared_ptr<GameTreeNode>& node, int depth, int index, int deal, uint64_t board) {
    switch (node->getType()) {
        case GameTreeNode::ACTION: {
            shared_ptr<ActionNode> action_node = std::dynamic_pointer_cast<ActionNode>(node);
            vector<shared_ptr<GameTreeNode>>& children = action_node->getChildrens();
            int owner = action_node->getPlayer();
            int n_actions = children.size();
            int train_off = levels[depth].train_size;
            levels[depth].train_size += (size_t)n_actions * n_hands[owner];
            if (levels[depth].train_size > (size_t)INT32_MAX)
                throw runtime_error("tree level too large for the OpenCL solver");

            int child_first = allocate(depth + 1, n_actions);
            levels[depth].actions.push_back({index, owner, n_actions, child_first, train_off, 0, 0, 0});
            trainable_index[{action_node.get(), deal}] = {depth, (int)write_back[depth].size()};
            write_back[depth].push_back({action_node.get(), deal, train_off, n_actions, owner});
            for (int a = 0; a < n_actions; a++)
                levels[depth + 1].pulls[child_first + a] = {index, 0, a, train_off, n_actions, owner, 1.0f, 0};
            for (int a = 0; a < n_actions; a++)
                expand(children[a], depth + 1, child_first + a, deal, board);
            break;
        }
        case GameTreeNode::CHANCE: {
            shared_ptr<ChanceNode> chance_node = std::dynamic_pointer_cast<ChanceNode>(node);
            const vector<Card>& cards = chance_node->getCards();
            int chance_cards = cards.size();
            if (chance_cards % 4 != 0)
                throw runtime_error("card num cannot round 4");
            int possible_deals = chance_cards - bitset<64>(board).count() - 2;

            vector<int> valid;
            for (int card = 0; card < chance_cards; card++) {
                int card_int = cards[card].getCardInt();
                if (Card::boardsHasIntercept(Card::boardInt2long(card_int), board))
                    continue;
                if (color_iso_offset[deal][card_int % 4] < 0)
                    continue;
                valid.push_back(card);
            }

            int child_first = allocate(depth + 1, valid.size());
            vector<int> child_of(chance_cards, -1);
            for (std::size_t j = 0; j < valid.size(); j++)
                child_of[valid[j]] = child_first + j;

            // Every card contributes, isomorphic ones through the child of the card they map to.
            Level& level = levels[depth];
            int contrib_first = level.contribs.size();
            for (int card = 0; card < chance_cards; card++) {
                int suit = cards[card].getCardInt() % 4;
                int offset = color_iso_offset[deal][suit];
                int source = card;
                int pair = -1;
                if (offset < 0) {
                    source = card + offset;
                    pair = suitPairIndex(suit, suit + offset);
                }
                if (source < 0 || source >= chance_cards || child_of[source] < 0)
                    continue;
                level.contribs.push_back({child_of[source], pair});
            }
            level.chances.push_back({index, contrib_first, (int)level.contribs.size() - contrib_first, 0});

            for (std::size_t j = 0; j < valid.size(); j++) {
                int card = valid[j];
                int card_int = cards[card].getCardInt();
                int new_deal;
                if (deal == 0) {
                    new_deal = card + 1;
                } else if (deal > 0 && deal <= chance_cards) {
                    new_deal = chance_cards * (deal - 1) + card + 1 + chance_cards;
                } else {
                    throw runtime_error(tfm::format("deal out of range : %s ", deal));
                }
                levels[depth + 1].pulls[child_first + j] = {index, 1, card_int, 0, 0, -1, (float)possible_deals, 0};
                expand(chance_node->getChildren(), depth + 1, child_first + j, new_deal,
                       board | Card::boardInt2long(card_int));
            }
            break;
        }
        case GameTreeNode::SHOWDOWN: {
            shared_ptr<ShowdownNode> showdown_node = std::dynamic_pointer_cast<ShowdownNode>(node);
            using R = ShowdownNode::ShowDownResult;
            levels[depth].showdowns.push_back({index, boardId(board),
                                               (float)showdown_node->get_payoffs(R::NOTTIE, 0, 0),
                                               (float)showdown_node->get_payoffs(R::NOTTIE, 1, 0),
                                               (float)showdown_node->get_payoffs(R::NOTTIE, 1, 1),
                                               (float)showdown_node->get_payoffs(R::NOTTIE, 0, 1), 0, 0});
            break;
        }
        case GameTreeNode::TERMINAL: {
            shared_ptr<TerminalNode> terminal_node = std::dynamic_pointer_cast<TerminalNode>(node);
            vector<double> payoffs = terminal_node->get_payoffs();
            levels[depth].terminals.push_back({index, (uint32_t)(board & 0xffffffffu), (uint32_t)(board >> 32),
                                               (float)payoffs[0], (float)payoffs[1], 0, 0, 0});
            break;
        }
        default:
            throw runtime_error("node type unknown");
    }
}

void GpuCfrEngine::buildHandTables() {
    hands.assign(2 * H, cl_int2{{0, 0}});
    same_index.assign(2 * H, -1);
    weights.assign(2 * H, 0.0f);
    br_oppo_sum.assign(2 * H, 0.0f);
    card_off.assign(2 * 53, 0);
    card_list.clear();
    perm.assign(2 * 6 * H, 0);

    for (int player = 0; player < 2; player++) {
        const vector<PrivateCards>& range = ranges[player];
        const vector<PrivateCards>& oppo_range = ranges[1 - player];
        vector<vector<int>> by_card(52);
        for (int h = 0; h < n_hands[player]; h++) {
            const PrivateCards& pc = range[h];
            hands[player * H + h] = cl_int2{{pc.card1, pc.card2}};
            same_index[player * H + h] = pcm.indPlayer2Player(player, 1 - player, h);
            weights[player * H + h] = pc.weight;
            by_card[pc.card1].push_back(h);
            by_card[pc.card2].push_back(h);

            uint64_t hand_long = Card::boardInt2long(pc.card1) | Card::boardInt2long(pc.card2);
            float oppo_sum = 0;
            for (const PrivateCards& oppo : oppo_range) {
                uint64_t oppo_long = Card::boardInt2long(oppo.card1) | Card::boardInt2long(oppo.card2);
                if (Card::boardsHasIntercept(hand_long, oppo_long) ||
                    Card::boardsHasIntercept(oppo_long, initial_board))
                    continue;
                oppo_sum += oppo.weight;
            }
            br_oppo_sum[player * H + h] = oppo_sum;
        }
        for (int c = 0; c < 52; c++) {
            card_off[player * 53 + c] = card_list.size();
            card_list.insert(card_list.end(), by_card[c].begin(), by_card[c].end());
        }
        card_off[player * 53 + 52] = card_list.size();

        // Hand permutations for swapping two suits, built with the CPU solver's exchange_color.
        for (int a = 0; a < 4; a++) {
            for (int b = a + 1; b < 4; b++) {
                vector<int> source(n_hands[player]);
                iota(source.begin(), source.end(), 0);
                exchange_color(source, range, a, b);
                copy(source.begin(), source.end(), perm.begin() + (player * 6 + suitPairIndex(a, b)) * H);
            }
        }
    }
}

void GpuCfrEngine::buildBoardTables() {
    size_t nb = boards.size();
    board_rank.assign(nb * 2 * H, -1);
    board_lt.assign(nb * 2 * H, 0);
    board_le.assign(nb * 2 * H, 0);
    board_sorted.assign(nb * 2 * H, 0);
    board_count.assign(nb * 2, 0);
    if (nb == 0)
        return;

    auto build = [this](size_t b) {
        uint64_t board = boards[b];
        int* rank[2] = {&board_rank[(b * 2) * H], &board_rank[(b * 2 + 1) * H]};
        for (int player = 0; player < 2; player++) {
            for (int h = 0; h < n_hands[player]; h++) {
                const PrivateCards& pc = ranges[player][h];
                uint64_t hand_long = Card::boardInt2long(pc.card1) | Card::boardInt2long(pc.card2);
                if (Card::boardsHasIntercept(hand_long, board))
                    continue;
                int same = same_index[player * H + h];
                if (player == 1 && same >= 0 && rank[0][same] >= 0)
                    rank[1][h] = rank[0][same];
                else
                    rank[player][h] = compairer->get_rank(hand_long, board);
            }
        }
        vector<int> sorted_ranks[2];
        for (int player = 0; player < 2; player++) {
            int* sorted = &board_sorted[(b * 2 + player) * H];
            int count = 0;
            for (int h = 0; h < n_hands[player]; h++)
                if (rank[player][h] >= 0)
                    sorted[count++] = h;
            stable_sort(sorted, sorted + count, [&](int x, int y) { return rank[player][x] < rank[player][y]; });
            board_count[b * 2 + player] = count;
            for (int k = 0; k < count; k++)
                sorted_ranks[player].push_back(rank[player][sorted[k]]);
        }
        for (int player = 0; player < 2; player++) {
            const vector<int>& oppo_ranks = sorted_ranks[1 - player];
            for (int h = 0; h < n_hands[player]; h++) {
                int r = rank[player][h];
                if (r < 0)
                    continue;
                board_lt[(b * 2 + player) * H + h] =
                    lower_bound(oppo_ranks.begin(), oppo_ranks.end(), r) - oppo_ranks.begin();
                board_le[(b * 2 + player) * H + h] =
                    upper_bound(oppo_ranks.begin(), oppo_ranks.end(), r) - oppo_ranks.begin();
            }
        }
    };

    size_t n_threads = max(1u, std::thread::hardware_concurrency());
    vector<std::thread> workers;
    for (size_t t = 0; t < n_threads; t++) {
        workers.emplace_back([&, t]() {
            for (size_t b = t; b < nb; b += n_threads)
                build(b);
        });
    }
    for (std::thread& worker : workers)
        worker.join();
}

void GpuCfrEngine::checkMemory() {
    size_t cum_bytes = use_halffloats ? 2 : 4;
    size_t total = bytesOf(hands) + bytesOf(same_index) + bytesOf(card_off) + bytesOf(card_list) + bytesOf(perm) +
                   bytesOf(board_rank) + bytesOf(board_lt) + bytesOf(board_le) + bytesOf(board_sorted) +
                   bytesOf(board_count);
    for (const Level& level : levels) {
        total += (size_t)level.n_inst * H * sizeof(float);
        total += level.train_size * (sizeof(float) + 2 * cum_bytes);
        total += bytesOf(level.pulls) + bytesOf(level.actions) + bytesOf(level.chances) + bytesOf(level.contribs) +
                 bytesOf(level.terminals) + bytesOf(level.showdowns);
    }
    qDebug().noquote() << QString::fromStdString(
        tfm::format("OpenCL solver needs %s MB of device memory", total / (1024 * 1024)));
    if ((double)total > 0.9 * (double)cl->globalMemSize())
        throw runtime_error(tfm::format("the tree needs %s MB but the device only has %s MB",
                                        total / (1024 * 1024), cl->globalMemSize() / (1024 * 1024)));
}

void GpuCfrEngine::buildKernels() {
    wg = 1;
    while (wg * 2 <= 256 && (size_t)(wg * 2) <= cl->maxWorkGroupSize())
        wg *= 2;
    size_t showdown_local = (4 * (size_t)H + 2 + 3 * (size_t)wg) * sizeof(float);
    if (showdown_local > cl->localMemSize())
        throw runtime_error(tfm::format("ranges of %s hands need more local memory than the device has", H));

    string options = tfm::format("-DWG=%s -DMAXH=%s -DCUM_HALF=%s", wg, H, use_halffloats ? 1 : 0);
    program = cl->buildProgram(kCfrKernelSource, options);
    k_pull = cl->createKernel(program, "pull_reach");
    k_terminal = cl->createKernel(program, "terminal_util");
    k_showdown = cl->createKernel(program, "showdown_util");
    k_chance = cl->createKernel(program, "chance_up");
    k_action = cl->createKernel(program, "action_up");
    k_ev = cl->createKernel(program, "action_ev");
}

cl_mem GpuCfrEngine::track(cl_mem buffer) {
    buffers.push_back(buffer);
    return buffer;
}

void GpuCfrEngine::upload() {
    hands_buf = track(cl->createBuffer(bytesOf(hands), hands.data()));
    same_buf = track(cl->createBuffer(bytesOf(same_index), same_index.data()));
    card_off_buf = track(cl->createBuffer(bytesOf(card_off), card_off.data()));
    card_list_buf = track(cl->createBuffer(bytesOf(card_list), card_list.data()));
    perm_buf = track(cl->createBuffer(bytesOf(perm), perm.data()));
    rank_buf = track(cl->createBuffer(bytesOf(board_rank), board_rank.data()));
    lt_buf = track(cl->createBuffer(bytesOf(board_lt), board_lt.data()));
    le_buf = track(cl->createBuffer(bytesOf(board_le), board_le.data()));
    sorted_buf = track(cl->createBuffer(bytesOf(board_sorted), board_sorted.data()));
    count_buf = track(cl->createBuffer(bytesOf(board_count), board_count.data()));
    // The board tables are only needed on the device.
    vector<int>().swap(board_rank);
    vector<int>().swap(board_lt);
    vector<int>().swap(board_le);
    vector<int>().swap(board_sorted);

    size_t cum_bytes = use_halffloats ? 2 : 4;
    const cl_uchar zero = 0;
    for (Level& level : levels) {
        size_t vector_bytes = (size_t)level.n_inst * H * sizeof(float);
        level.vec = track(cl->createBuffer(vector_bytes));
        level.rplus = track(cl->createBuffer(level.train_size * sizeof(float)));
        level.cum = track(cl->createBuffer(level.train_size * cum_bytes));
        level.ev = track(cl->createBuffer(level.train_size * cum_bytes));
        for (cl_mem buffer : {level.vec, level.rplus, level.cum, level.ev}) {
            size_t size = 0;
            clCheck(clGetMemObjectInfo(buffer, CL_MEM_SIZE, sizeof(size), &size, nullptr), "clGetMemObjectInfo");
            clCheck(clEnqueueFillBuffer(cl->queue(), buffer, &zero, 1, 0, size, 0, nullptr, nullptr),
                    "clEnqueueFillBuffer");
        }
        level.pulls_buf = track(cl->createBuffer(bytesOf(level.pulls), level.pulls.data()));
        level.actions_buf = track(cl->createBuffer(bytesOf(level.actions), level.actions.data()));
        level.chances_buf = track(cl->createBuffer(bytesOf(level.chances), level.chances.data()));
        level.contribs_buf = track(cl->createBuffer(bytesOf(level.contribs), level.contribs.data()));
        level.terminals_buf = track(cl->createBuffer(bytesOf(level.terminals), level.terminals.data()));
        level.showdowns_buf = track(cl->createBuffer(bytesOf(level.showdowns), level.showdowns.data()));
        vector<PullDesc>().swap(level.pulls);
        vector<Contrib>().swap(level.contribs);
    }
    cl->finish();
}

void GpuCfrEngine::launch2d(cl_kernel kernel, int width, int height) {
    if (width <= 0 || height <= 0)
        return;
    size_t global[2] = {(size_t)(width + kGroup2d - 1) / kGroup2d * kGroup2d, (size_t)height};
    size_t local[2] = {(size_t)kGroup2d, 1};
    clCheck(clEnqueueNDRangeKernel(cl->queue(), kernel, 2, nullptr, global, local, 0, nullptr, nullptr),
            "clEnqueueNDRangeKernel");
}

void GpuCfrEngine::launchGroups(cl_kernel kernel, int groups) {
    if (groups <= 0)
        return;
    size_t global = (size_t)groups * wg;
    size_t local = wg;
    clCheck(clEnqueueNDRangeKernel(cl->queue(), kernel, 1, nullptr, &global, &local, 0, nullptr, nullptr),
            "clEnqueueNDRangeKernel");
}

void GpuCfrEngine::traverse(int player, Mode mode, bool update, bool compute_ev, int iter) {
    int opp = 1 - player;
    int n_player = n_hands[player];
    int n_opp = n_hands[opp];
    int use_average = mode == BEST_RESPONSE ? 1 : 0;
    int mode_arg = mode;
    int update_arg = update ? 1 : 0;

    // Discounted CFR coefficients, as in DiscountedCfrTrainable::updateRegrets.
    int iteration_number = iter + 1;
    double alpha_pow = pow(iteration_number, 1.5);
    float alpha_coef = (float)(alpha_pow / (1 + alpha_pow));
    float strategy_coef = pow((float)iteration_number / (iteration_number + 1), 2.0f);

    cl->write(levels[0].vec, &weights[opp * H], n_opp * sizeof(float));

    for (std::size_t d = 1; d < levels.size(); d++) {
        Level& parent = levels[d - 1];
        Level& level = levels[d];
        setKernelArgs(k_pull, parent.vec, level.vec, level.pulls_buf, level.n_inst, H, n_opp, opp, hands_buf,
                      parent.rplus, parent.cum, use_average);
        launch2d(k_pull, n_opp, level.n_inst);
    }

    for (int d = (int)levels.size() - 1; d >= 0; d--) {
        Level& level = levels[d];
        if (!level.terminals.empty()) {
            setKernelArgs(k_terminal, level.vec, level.terminals_buf, H, player, n_player, n_opp,
                          hands_buf, same_buf, card_off_buf, card_list_buf);
            launchGroups(k_terminal, level.terminals.size());
        }
        if (!level.showdowns.empty()) {
            setKernelArgs(k_showdown, level.vec, level.showdowns_buf, H, player, n_player, n_opp,
                          hands_buf, card_off_buf, card_list_buf, rank_buf, lt_buf, le_buf, sorted_buf, count_buf);
            launchGroups(k_showdown, level.showdowns.size());
        }
        if (d + 1 >= (int)levels.size())
            continue;
        Level& next = levels[d + 1];
        if (!level.chances.empty()) {
            int n_chances = level.chances.size();
            setKernelArgs(k_chance, next.vec, level.vec, level.chances_buf, n_chances, level.contribs_buf,
                          perm_buf, H, player, n_player);
            launch2d(k_chance, n_player, n_chances);
        }
        if (!level.actions.empty()) {
            int n_actions = level.actions.size();
            if (compute_ev) {
                // Reads this level's reach, so it must run before action_up overwrites it with utilities.
                setKernelArgs(k_ev, level.vec, next.vec, level.actions_buf, H, player, n_player, n_opp,
                              hands_buf, same_buf, card_off_buf, card_list_buf, level.ev);
                launchGroups(k_ev, n_actions);
            }
            setKernelArgs(k_action, next.vec, level.vec, level.actions_buf, n_actions, H, player, n_player,
                          mode_arg, update_arg, alpha_coef, strategy_coef, level.rplus, level.cum);
            launch2d(k_action, n_player, n_actions);
        }
    }
}

void GpuCfrEngine::iterate(int iter, bool compute_ev) {
    for (int player = 0; player < 2; player++)
        traverse(player, CFR, true, compute_ev, iter);
    cl->finish();
}

void GpuCfrEngine::collectStatistics(int iter) {
    for (int player = 0; player < 2; player++)
        traverse(player, CFR, false, true, iter);
    cl->finish();
}

// Same report as BestResponse::printExploitability.
float GpuCfrEngine::exploitability(int iteration_count, float initial_pot) {
    qDebug().noquote() << QString::fromStdString(
        tfm::format(QObject::tr("Iter: %s").toStdString().c_str(), iteration_count));
    float exploitible = 0;
    for (int player = 0; player < 2; player++) {
        traverse(player, BEST_RESPONSE, false, false, 0);
        vector<float> evs(n_hands[player]);
        cl->read(levels[0].vec, evs.data(), evs.size() * sizeof(float));

        const vector<PrivateCards>& combos = pcm.getPreflopCards(player);
        float ev = 0;
        for (int h = 0; h < n_hands[player]; h++) {
            const PrivateCards& pc = combos[h];
            uint64_t hand_long = Card::boardInt2long(pc.card1) | Card::boardInt2long(pc.card2);
            if (Card::boardsHasIntercept(hand_long, initial_board))
                continue;
            ev += evs[h] * pc.relative_prob / br_oppo_sum[player * H + h];
        }
        exploitible += ev;
        qDebug().noquote() << QString::fromStdString(
            tfm::format(QObject::tr("player %s exploitability %s").toStdString().c_str(), player, ev));
    }
    float total_exploitability = exploitible / 2 / initial_pot * 100;
    qDebug().noquote() << QString::fromStdString(
        tfm::format(QObject::tr("Total exploitability %s precent").toStdString().c_str(), total_exploitability));
    return total_exploitability;
}

// Reads values of a trainable buffer, converting fp16 storage to float.
vector<float> GpuCfrEngine::readValues(cl_mem buffer, bool half, size_t offset, size_t count) {
    vector<float> values(count);
    if (half) {
        vector<uint16_t> raw(count);
        cl->read(buffer, raw.data(), count * sizeof(uint16_t), offset * sizeof(uint16_t));
        transform(raw.begin(), raw.end(), values.begin(), halfToFloat);
    } else {
        cl->read(buffer, values.data(), count * sizeof(float), offset * sizeof(float));
    }
    return values;
}

void GpuCfrEngine::syncTrainable(ActionNode* node, int deal) {
    auto it = trainable_index.find({node, deal});
    if (it == trainable_index.end())
        return;
    Level& level = levels[it->second.first];
    const WriteBackItem& item = write_back[it->second.first][it->second.second];
    size_t count = (size_t)item.n_actions * n_hands[item.owner];
    bool half = use_halffloats != 0;
    node->getTrainable(deal, true, use_halffloats)
        ->loadState(readValues(level.rplus, false, item.train_off, count),
                    readValues(level.cum, half, item.train_off, count),
                    readValues(level.ev, half, item.train_off, count));
}

void GpuCfrEngine::writeBack() {
    for (std::size_t d = 0; d < levels.size(); d++) {
        if (write_back[d].empty())
            continue;
        Level& level = levels[d];
        size_t n = level.train_size;
        vector<float> rplus = readValues(level.rplus, false, 0, n);
        vector<float> cum = readValues(level.cum, use_halffloats != 0, 0, n);
        vector<float> ev = readValues(level.ev, use_halffloats != 0, 0, n);
        for (const WriteBackItem& item : write_back[d]) {
            size_t begin = item.train_off;
            size_t end = begin + (size_t)item.n_actions * n_hands[item.owner];
            shared_ptr<Trainable> trainable = item.node->getTrainable(item.deal, true, use_halffloats);
            trainable->loadState(vector<float>(rplus.begin() + begin, rplus.begin() + end),
                                 vector<float>(cum.begin() + begin, cum.begin() + end),
                                 vector<float>(ev.begin() + begin, ev.begin() + end));
        }
    }
}
