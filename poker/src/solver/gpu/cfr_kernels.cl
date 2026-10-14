// OpenCL kernels for the discounted CFR solver (GpuCfrEngine).
//
// The game tree is expanded into "instances" (one per tree node and card deal) and grouped by depth.
// Each instance owns one per-hand row (stride H) in its level's vector buffer: the top-down pass fills it with the
// opponent's reach probabilities, and the bottom-up pass overwrites it with the traverser's utilities.
// Trainable state (regrets, cumulative strategy, evs) uses the CPU layout [action * hands_of_owner + hand].
//
// Build options:
//   -DWG=<n>        work-group size of the per-instance kernels, a power of two
//   -DMAXH=<n>      largest hand count of the two players
//   -DCUM_HALF=0|1  store cumulative strategy and evs as fp16

#if CUM_HALF
#define CUM_T half
#define LOAD_CUM(p, i) vload_half((i), (p))
#define STORE_CUM(p, i, v) vstore_half((v), (i), (p))
#else
#define CUM_T float
#define LOAD_CUM(p, i) ((p)[i])
#define STORE_CUM(p, i, v) ((p)[i] = (v))
#endif

#define DCFR_BETA 0.5f
#define DCFR_THETA 0.9f

typedef struct {
    int parent;     // instance index in the previous level
    int kind;       // 0: action edge, 1: chance edge
    int arg;        // action index, or the dealt card
    int train_off;  // action edge: parent's trainable offset
    int n_actions;  // action edge: parent's action count
    int owner;      // action edge: player acting at the parent
    float divisor;  // chance edge: number of possible deals
    int pad;
} PullDesc;

typedef struct {
    int inst;
    int owner;
    int n_actions;
    int child_first;  // children are contiguous in the next level
    int train_off;
    int pad0;
    int pad1;
    int pad2;
} ActionDesc;

typedef struct {
    int inst;
    int contrib_first;
    int contrib_count;
    int pad;
} ChanceDesc;

typedef struct {
    int child;  // child instance in the next level
    int pair;   // suit pair to exchange (isomorphic deal), -1 for none
} Contrib;

typedef struct {
    int inst;
    uint board_lo;
    uint board_hi;
    float payoff0;
    float payoff1;
    int pad0;
    int pad1;
    int pad2;
} TerminalDesc;

typedef struct {
    int inst;
    int board_id;
    float win0;
    float lose0;
    float win1;
    float lose1;
    int pad0;
    int pad1;
} ShowdownDesc;

float strategy_prob(__global const float* rplus, __global const CUM_T* cum, int base, int n_actions, int n_hands,
                    int action, int hand, int use_average) {
    float sum = 0.0f;
    if (use_average) {
        for (int a = 0; a < n_actions; a++)
            sum += LOAD_CUM(cum, base + a * n_hands + hand);
        if (sum != 0.0f)
            return LOAD_CUM(cum, base + action * n_hands + hand) / sum;
    } else {
        for (int a = 0; a < n_actions; a++)
            sum += fmax(0.0f, rplus[base + a * n_hands + hand]);
        if (sum != 0.0f)
            return fmax(0.0f, rplus[base + action * n_hands + hand]) / sum;
    }
    return 1.0f / n_actions;
}

// Copies an instance's opponent reach into local memory, then computes its card sums (card_sum[c] = reach of the
// opponent hands holding c) and total. Must be called by the whole work-group; afterwards the global row may be
// overwritten.
float opponent_card_sums(__global const float* row, int n_opp, int opp, __global const int* card_off,
                         __global const int* card_list, __local float* rp, __local float* card_sum,
                         __local float* red) {
    int lid = get_local_id(0);
    for (int h = lid; h < n_opp; h += WG)
        rp[h] = row[h];
    barrier(CLK_LOCAL_MEM_FENCE);
    for (int c = lid; c < 52; c += WG) {
        float s = 0.0f;
        int end = card_off[opp * 53 + c + 1];
        for (int k = card_off[opp * 53 + c]; k < end; k++)
            s += rp[card_list[k]];
        card_sum[c] = s;
    }
    float s = 0.0f;
    for (int h = lid; h < n_opp; h += WG)
        s += rp[h];
    red[lid] = s;
    barrier(CLK_LOCAL_MEM_FENCE);
    for (int stride = WG / 2; stride > 0; stride >>= 1) {
        if (lid < stride)
            red[lid] += red[lid + stride];
        barrier(CLK_LOCAL_MEM_FENCE);
    }
    return red[0];
}

// Top-down: opponent reach of every instance in a level from its parent.
__kernel void pull_reach(__global const float* parent_vec, __global float* vec, __global const PullDesc* pulls,
                         int n_inst, int H, int n_opp, int opp, __global const int2* hands,
                         __global const float* parent_rplus, __global const CUM_T* parent_cum, int use_average) {
    int h = get_global_id(0);
    int i = get_global_id(1);
    if (h >= n_opp || i >= n_inst)
        return;
    PullDesc pd = pulls[i];
    float r = parent_vec[(size_t)pd.parent * H + h];
    if (pd.kind == 1) {
        int2 hc = hands[opp * H + h];
        r = (hc.x == pd.arg || hc.y == pd.arg) ? 0.0f : r / pd.divisor;
    } else if (pd.owner == opp) {
        r *= strategy_prob(parent_rplus, parent_cum, pd.train_off, pd.n_actions, n_opp, pd.arg, h, use_average);
    }
    vec[(size_t)i * H + h] = r;
}

// Fold payoffs. One work-group per terminal instance.
__kernel void terminal_util(__global float* vec, __global const TerminalDesc* terms, int H, int player, int n_player,
                            int n_opp, __global const int2* hands, __global const int* same_index,
                            __global const int* card_off, __global const int* card_list) {
    __local float rp[MAXH];
    __local float card_sum[52];
    __local float red[WG];
    TerminalDesc td = terms[get_group_id(0)];
    int lid = get_local_id(0);
    int opp = 1 - player;
    __global float* out = vec + (size_t)td.inst * H;
    float total = opponent_card_sums(out, n_opp, opp, card_off, card_list, rp, card_sum, red);

    ulong board = ((ulong)td.board_hi << 32) | (ulong)td.board_lo;
    float payoff = player == 0 ? td.payoff0 : td.payoff1;
    for (int h = lid; h < n_player; h += WG) {
        int2 hc = hands[player * H + h];
        ulong hand_mask = (1UL << hc.x) | (1UL << hc.y);
        float u = 0.0f;
        if ((hand_mask & board) == 0) {
            int same = same_index[player * H + h];
            float plus = same >= 0 ? rp[same] : 0.0f;
            u = payoff * (total - card_sum[hc.x] - card_sum[hc.y] + plus);
        }
        out[h] = u;
    }
}

// Showdown payoffs. One work-group per showdown instance.
// For every river board: board_rank holds each hand's rank (-1 when blocked, lower is stronger), board_sorted the
// valid hands in ascending rank order, and board_lt / board_le how many opponent hands rank strictly below / at most
// the hand's rank.
__kernel void showdown_util(__global float* vec, __global const ShowdownDesc* sds, int H, int player, int n_player, int n_opp, __global const int2* hands,
                            __global const int* card_off, __global const int* card_list,
                            __global const int* board_rank, __global const int* board_lt,
                            __global const int* board_le, __global const int* board_sorted,
                            __global const int* board_count) {
    __local float lreach[MAXH];
    __local int lrank[MAXH];
    __local float prefix[MAXH + 1];
    __local float suffix[MAXH + 1];
    __local float chunk_sum[WG];
    __local float part[WG];
    __local float part_suffix[WG];

    ShowdownDesc sd = sds[get_group_id(0)];
    int lid = get_local_id(0);
    int opp = 1 - player;
    __global float* out = vec + (size_t)sd.inst * H;
    size_t opp_base = ((size_t)sd.board_id * 2 + opp) * H;
    size_t player_base = ((size_t)sd.board_id * 2 + player) * H;

    for (int h = lid; h < n_opp; h += WG) {
        lreach[h] = out[h];
        lrank[h] = board_rank[opp_base + h];
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    // Prefix and suffix sums of the opponent reach in ascending rank order. Both are accumulated directly, like the
    // CPU solver does, rather than derived from each other, which would lose precision for small sums.
    int count = board_count[sd.board_id * 2 + opp];
    int chunk = (count + WG - 1) / WG;
    int begin = min(lid * chunk, count);
    int end = min(begin + chunk, count);
    float s = 0.0f;
    for (int k = begin; k < end; k++)
        s += lreach[board_sorted[opp_base + k]];
    chunk_sum[lid] = s;
    barrier(CLK_LOCAL_MEM_FENCE);
    if (lid == 0) {
        float run = 0.0f;
        for (int t = 0; t < WG; t++) {
            part[t] = run;
            run += chunk_sum[t];
        }
        prefix[count] = run;
        run = 0.0f;
        for (int t = WG - 1; t >= 0; t--) {
            part_suffix[t] = run;
            run += chunk_sum[t];
        }
        suffix[count] = 0.0f;
    }
    barrier(CLK_LOCAL_MEM_FENCE);
    float run = part[lid];
    for (int k = begin; k < end; k++) {
        prefix[k] = run;
        run += lreach[board_sorted[opp_base + k]];
    }
    run = part_suffix[lid];
    for (int k = end - 1; k >= begin; k--) {
        run += lreach[board_sorted[opp_base + k]];
        suffix[k] = run;
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    float win = player == 0 ? sd.win0 : sd.win1;
    float lose = player == 0 ? sd.lose0 : sd.lose1;
    for (int h = lid; h < n_player; h += WG) {
        int r = board_rank[player_base + h];
        float u = 0.0f;
        if (r >= 0) {
            float beaten = suffix[board_le[player_base + h]];
            float beating = prefix[board_lt[player_base + h]];
            // Remove opponent hands sharing a card with ours.
            int2 hc = hands[player * H + h];
            for (int c = 0; c < 2; c++) {
                int card = c == 0 ? hc.x : hc.y;
                int stop = card_off[opp * 53 + card + 1];
                for (int k = card_off[opp * 53 + card]; k < stop; k++) {
                    int j = card_list[k];
                    int rj = lrank[j];
                    if (rj < 0)
                        continue;
                    if (rj > r)
                        beaten -= lreach[j];
                    else if (rj < r)
                        beating -= lreach[j];
                }
            }
            u = beaten * win + beating * lose;
        }
        out[h] = u;
    }
}

// Bottom-up: chance node utility, summing over every dealt card (isomorphic deals reuse a child with suits swapped).
__kernel void chance_up(__global const float* child_vec, __global float* vec, __global const ChanceDesc* cds,
                        int n_cd, __global const Contrib* contribs, __global const int* perm, int H, int player,
                        int n_player) {
    int h = get_global_id(0);
    int i = get_global_id(1);
    if (h >= n_player || i >= n_cd)
        return;
    ChanceDesc cd = cds[i];
    float u = 0.0f;
    for (int k = 0; k < cd.contrib_count; k++) {
        Contrib c = contribs[cd.contrib_first + k];
        int src = c.pair < 0 ? h : perm[(player * 6 + c.pair) * H + h];
        u += child_vec[(size_t)c.child * H + src];
    }
    vec[(size_t)cd.inst * H + h] = u;
}

// Bottom-up: action node utility.
// mode 0 (CFR): the traverser plays its current strategy and, when update is set, the discounted CFR update runs.
// mode 1 (best response): the traverser picks its best action.
__kernel void action_up(__global const float* child_vec, __global float* vec, __global const ActionDesc* ads,
                        int n_ad, int H, int player, int n_player, int mode, int update, float alpha_coef,
                        float strategy_coef, __global float* rplus, __global CUM_T* cum) {
    int h = get_global_id(0);
    int i = get_global_id(1);
    if (h >= n_player || i >= n_ad)
        return;
    ActionDesc ad = ads[i];
    int n_actions = ad.n_actions;
    __global const float* cu = child_vec + (size_t)ad.child_first * H + h;
    float u = 0.0f;
    if (ad.owner != player) {
        for (int a = 0; a < n_actions; a++)
            u += cu[(size_t)a * H];
    } else if (mode == 1) {
        u = cu[0];
        for (int a = 1; a < n_actions; a++)
            u = max(u, cu[(size_t)a * H]);
    } else {
        int base = ad.train_off + h;
        float sum = 0.0f;
        for (int a = 0; a < n_actions; a++)
            sum += fmax(0.0f, rplus[base + a * n_player]);
        for (int a = 0; a < n_actions; a++) {
            float s = sum != 0.0f ? fmax(0.0f, rplus[base + a * n_player]) / sum : 1.0f / n_actions;
            u += s * cu[(size_t)a * H];
        }
        if (update) {
            float new_sum = 0.0f;
            for (int a = 0; a < n_actions; a++) {
                float r = rplus[base + a * n_player] + (cu[(size_t)a * H] - u);
                r *= r > 0.0f ? alpha_coef : DCFR_BETA;
                rplus[base + a * n_player] = r;
                new_sum += fmax(0.0f, r);
            }
            for (int a = 0; a < n_actions; a++) {
                int index = base + a * n_player;
                float s = new_sum != 0.0f ? fmax(0.0f, rplus[index]) / new_sum : 1.0f / n_actions;
                STORE_CUM(cum, index, LOAD_CUM(cum, index) * DCFR_THETA + s * strategy_coef);
            }
        }
    }
    vec[(size_t)ad.inst * H + h] = u;
}

// Per action evs of the traverser's hands. One work-group per action instance; must run before action_up replaces
// the instance's reach with its utility.
__kernel void action_ev(__global const float* vec, __global const float* child_vec, __global const ActionDesc* ads,
                        int H, int player, int n_player, int n_opp, __global const int2* hands,
                        __global const int* same_index, __global const int* card_off,
                        __global const int* card_list, __global CUM_T* ev) {
    __local float rp[MAXH];
    __local float card_sum[52];
    __local float red[WG];
    ActionDesc ad = ads[get_group_id(0)];
    if (ad.owner != player)
        return;
    int lid = get_local_id(0);
    float total = opponent_card_sums(vec + (size_t)ad.inst * H, n_opp, 1 - player, card_off, card_list, rp,
                                     card_sum, red);

    for (int h = lid; h < n_player; h += WG) {
        int2 hc = hands[player * H + h];
        int same = same_index[player * H + h];
        float plus = same >= 0 ? rp[same] : 0.0f;
        float rp_sum = total - card_sum[hc.x] - card_sum[hc.y] + plus;
        for (int a = 0; a < ad.n_actions; a++) {
            float e = child_vec[(size_t)(ad.child_first + a) * H + h] / rp_sum;
            if (!isnan(e))
                STORE_CUM(ev, ad.train_off + a * n_player + h, e);
        }
    }
}
