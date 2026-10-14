#ifndef TEXASSOLVER_GPUCFRENGINE_H
#define TEXASSOLVER_GPUCFRENGINE_H

#include <include/Deck.h>
#include <include/GameTree.h>
#include <include/compairer/Compairer.h>
#include <include/ranges/PrivateCards.h>
#include <include/ranges/PrivateCardsManager.h>
#include <map>
#include <memory>
#include <unordered_map>
#include "src/solver/gpu/OpenClContext.h"

// Discounted CFR on an OpenCL device.
//
// The tree is expanded once into instances (tree node x card deal) grouped by depth. Each traversal walks the
// levels top-down to push the opponent's reach probabilities, then bottom-up to compute utilities and update the
// trainables, with one kernel launch per node type and level that covers every instance and hand at once.
// Produces the same results as PCfrSolver's recursive CPU traversal, up to float rounding.
class GpuCfrEngine {
public:
    GpuCfrEngine(const shared_ptr<GameTree>& tree, const vector<vector<PrivateCards>>& ranges,
                 PrivateCardsManager& pcm, const shared_ptr<Compairer>& compairer, Deck& deck,
                 const int color_iso_offset[][4], int use_halffloats, uint64_t initial_board);
    ~GpuCfrEngine();
    GpuCfrEngine(const GpuCfrEngine&) = delete;
    GpuCfrEngine& operator=(const GpuCfrEngine&) = delete;

    // True when the user asked for the CPU solver (TEXASSOLVER_DISABLE_OPENCL=1).
    static bool disabledByEnvironment();

    string description() const;
    // One discounted CFR iteration: a regret update for each player in turn. With compute_ev the per action evs
    // are refreshed as well, like the CPU solver does on print intervals.
    void iterate(int iter, bool compute_ev);
    // Prints and returns the exploitability of the average strategy, in percent of the pot.
    float exploitability(int iteration_count, float initial_pot);
    // Computes the per action evs of the current strategy without updating the regrets.
    void collectStatistics(int iter);
    // Copies regrets, average strategy and evs into the ActionNode trainables, where dumps and queries read them.
    void writeBack();
    // Copies one trainable's current state from the device, so the strategy can be inspected while solving.
    // Safe to call from another thread than the one running the solve.
    void syncTrainable(ActionNode* node, int deal);

    // Device side mirrors of the structs in cfr_kernels.cl.
    struct PullDesc {
        int parent, kind, arg, train_off, n_actions, owner;
        float divisor;
        int pad;
    };
    struct ActionDesc {
        int inst, owner, n_actions, child_first, train_off, pad0, pad1, pad2;
    };
    struct ChanceDesc {
        int inst, contrib_first, contrib_count, pad;
    };
    struct Contrib {
        int child, pair;
    };
    struct TerminalDesc {
        int inst;
        uint32_t board_lo, board_hi;
        float payoff0, payoff1;
        int pad0, pad1, pad2;
    };
    struct ShowdownDesc {
        int inst, board_id;
        float win0, lose0, win1, lose1;
        int pad0, pad1;
    };

private:
    struct Level {
        int n_inst = 0;
        size_t train_size = 0;
        vector<PullDesc> pulls;
        vector<ActionDesc> actions;
        vector<ChanceDesc> chances;
        vector<Contrib> contribs;
        vector<TerminalDesc> terminals;
        vector<ShowdownDesc> showdowns;
        // Per instance row: the opponent reach on the way down, the traverser's utility on the way up.
        cl_mem vec = nullptr, rplus = nullptr, cum = nullptr, ev = nullptr;
        cl_mem pulls_buf = nullptr, actions_buf = nullptr, chances_buf = nullptr, contribs_buf = nullptr,
               terminals_buf = nullptr, showdowns_buf = nullptr;
    };
    struct WriteBackItem {
        ActionNode* node;
        int deal, train_off, n_actions, owner;
    };
    enum Mode { CFR = 0, BEST_RESPONSE = 1 };

    int allocate(int depth, int count);
    void expand(const shared_ptr<GameTreeNode>& node, int depth, int index, int deal, uint64_t board);
    int boardId(uint64_t board);
    void buildHandTables();
    void buildBoardTables();
    void checkMemory();
    void upload();
    void buildKernels();
    void traverse(int player, Mode mode, bool update, bool compute_ev, int iter);
    void launch2d(cl_kernel kernel, int width, int height);
    void launchGroups(cl_kernel kernel, int groups);
    vector<float> readValues(cl_mem buffer, bool half, size_t offset, size_t count);
    cl_mem track(cl_mem buffer);

    Deck& deck;
    PrivateCardsManager& pcm;
    shared_ptr<Compairer> compairer;
    vector<vector<PrivateCards>> ranges;
    const int (*color_iso_offset)[4];
    int use_halffloats;
    uint64_t initial_board;
    int card_num;
    int n_hands[2];
    int H;
    int wg = 0;

    vector<Level> levels;
    vector<vector<WriteBackItem>> write_back;  // per level
    map<pair<ActionNode*, int>, pair<int, int>> trainable_index;  // (node, deal) -> (level, write_back index)
    unordered_map<uint64_t, int> board_ids;
    vector<uint64_t> boards;

    // Hand tables, per player with a stride of H.
    vector<cl_int2> hands;
    vector<int> same_index;
    vector<int> card_off;  // per player, 53 offsets into card_list
    vector<int> card_list;
    vector<int> perm;      // per player and suit pair
    vector<float> weights;
    vector<float> br_oppo_sum;  // per player hand: weight of the opponent hands it does not block
    vector<int> board_rank, board_lt, board_le, board_sorted, board_count;

    unique_ptr<OpenClContext> cl;
    cl_program program = nullptr;
    cl_kernel k_pull = nullptr, k_terminal = nullptr, k_showdown = nullptr, k_chance = nullptr, k_action = nullptr,
              k_ev = nullptr;
    cl_mem hands_buf = nullptr, same_buf = nullptr, card_off_buf = nullptr, card_list_buf = nullptr,
           perm_buf = nullptr, rank_buf = nullptr, lt_buf = nullptr, le_buf = nullptr, sorted_buf = nullptr,
           count_buf = nullptr;
    vector<cl_mem> buffers;
};

#endif //TEXASSOLVER_GPUCFRENGINE_H
