const game = {
  label: "Texas Hold'em",
  ranks: "2,3,4,5,6,7,8,9,T,J,Q,K,A",
  cards: "52 cards",
  deckSize: "52",
  combos: "1,326 hole-card combos",
  table: "2,598,961 five-card ranks",
  file: "card5_dic_sorted.txt"
};

const steps = [
  {
    id: "load",
    title: "Load the one solver",
    pipe: "Deck",
    text: "The server holds a single PokerSolver. The load command builds it once from the 13 Hold'em ranks, the four suits and the Hold'em hand-ranking file. Every later build, solve and strategy read uses that object's deck and comparer.",
    callout: (g) => `${g.label} uses ranks ${g.ranks}. Loading happens once per server process; a second load is ignored.`,
    file: "src/api/local_backend.cpp",
    where: "LocalBackend::load",
    code: `void LocalBackend::load() {
    clearNodeMap();

    const string suits = "c,d,h,s";
    const string resource_dir = ":/resources";

    emitProgress("Loading holdem comparing file");
    solver_ = PokerSolver("2,3,4,5,6,7,8,9,T,J,Q,K,A", suits,
                          resource_dir + "/compairer/card5_dic_sorted.txt", 2598961,
                          resource_dir + "/compairer/card5_dic_zipped.bin");

    emitProgress("Loading finished. Good to go.");
}`
  },
  {
    id: "deck",
    title: "Build the deck and the rank table",
    pipe: "Deck",
    text: "PokerSolver splits the rank string and suit string into a Deck, then loads Dic5Compairer. The comparer maps each five-card set to an integer. A smaller integer is a stronger hand. Showdown never reimplements poker rules; it only compares those integers.",
    callout: (g) => `${g.label}: ${g.cards}, lookup file ${g.file}, ${g.table}.`,
    file: "src/runtime/PokerSolver.cpp",
    where: "PokerSolver::PokerSolver",
    code: `PokerSolver::PokerSolver(string ranks, string suits, string compairer_file,
                         int compairer_file_lines, string compairer_file_bin) {
    vector<string> ranks_vector = string_split(ranks, ',');
    vector<string> suits_vector = string_split(suits, ',');
    this->deck = Deck(ranks_vector, suits_vector);
    this->compairer = make_shared<Dic5Compairer>(compairer_file, compairer_file_lines, compairer_file_bin);
}`
  },
  {
    id: "tree",
    title: "Build the betting tree",
    pipe: "Tree",
    text: "build_game_tree turns the bet sizes, pot, stack, raise limit and all-in threshold into a GameTree. Action nodes are decisions. Chance nodes deal the next street. Terminal nodes are folds. Showdown nodes are calls on the river. The tree only stores betting; the cards a chance node may deal come from the deck.",
    callout: (g) => `A turn chance node deals one of the remaining cards from the ${g.deckSize}-card deck, skipping the board and the two hole cards.`,
    file: "src/runtime/PokerSolver.cpp",
    where: "PokerSolver::build_game_tree",
    code: `void PokerSolver::build_game_tree(float oop_commit, float ip_commit, int current_round, int raise_limit,
                                  float small_blind, float big_blind, float stack,
                                  GameTreeBuildingSettings buildingSettings, float allin_threshold) {
    shared_ptr<GameTree> game_tree =
        make_shared<GameTree>(this->deck, oop_commit, ip_commit, current_round, raise_limit, small_blind, big_blind,
                              stack, buildingSettings, allin_threshold);
    this->game_tree = game_tree;
}`
  },
  {
    id: "ranges",
    title: "Turn range text into hands",
    pipe: "Ranges",
    text: "train() parses both ranges against the current board, then drops any hand that uses a board card. Those two vectors are the only hands CFR will score. A hand that shares a card with the opponent is removed later, inside the payoff, by subtracting that opponent's reach.",
    callout: (g) => `${g.label} starts from ${g.combos}. Hands that collide with the board are removed before the first iteration.`,
    file: "src/runtime/PokerSolver.cpp",
    where: "PokerSolver::train and noDuplicateRange",
    code: `vector<PrivateCards> range1 = PrivateRangeConverter::rangeStr2Cards(player1RangeStr, initialBoard);
vector<PrivateCards> range2 = PrivateRangeConverter::rangeStr2Cards(player2RangeStr, initialBoard);
uint64_t initial_board_long = Card::boardInts2long(initialBoard);

this->player1Range = noDuplicateRange(range1, initial_board_long);
this->player2Range = noDuplicateRange(range2, initial_board_long);

this->solver = make_shared<PCfrSolver>(game_tree, range1, range2, initialBoard, compairer, deck,
                                       iteration_number, false, print_interval, logfile_name,
                                       algorithm, Solver::MonteCarolAlg::NONE, warmup, accuracy,
                                       use_isomorphism, use_halffloats, threads);
this->solver->train();`
  },
  {
    id: "trainable",
    title: "Give every decision its own regrets",
    pipe: "Ranges",
    text: "setTrainable walks the tree. Each action node gets one DiscountedCfrTrainable per board deal that can reach it. On the root street that is 1 slot. One street later it is deck size + 1. Two streets later it is deck² + deck + 1. Chance nodes have no regrets; they only pass the walk to their child.",
    callout: (g) => `With ${g.cards}, a river decision reached from the flop keeps ${g.deckSize} × ${g.deckSize} + ${g.deckSize} + 1 = 2,757 trainable slots.`,
    file: "src/solver/PCfrSolver.cpp",
    where: "PCfrSolver::setTrainable",
    code: `if (this->trainer == "discounted_cfr") {
    int gap = current_round - root_round;
    if (gap == 2)
        num = this->deck.getCards().size() * this->deck.getCards().size() + this->deck.getCards().size() + 1;
    else if (gap == 1)
        num = this->deck.getCards().size() + 1;
    else if (gap == 0)
        num = 1;
    action_node->setTrainable(vector<shared_ptr<Trainable>>(num), player_privates);
}`
  },
  {
    id: "loop",
    title: "One iteration updates both players",
    pipe: "CFR",
    text: "train() loops up to max_iteration. Each iteration calls cfr once for IP and once for OOP. The reach vector passed in belongs to the opponent: it is the probability of arriving at this node with each of the opponent's hands. After the warmup, every print_interval iterations BestResponse measures exploitability and the loop can stop early.",
    callout: () => "IP and OOP take turns as the traverser. The opponent's strategy is held fixed during that player's walk.",
    file: "src/solver/PCfrSolver.cpp",
    where: "PCfrSolver::train",
    code: `for (int i = 0; i < this->iteration_number; i++) {
    for (int player_id = 0; player_id < this->player_number; player_id++) {
        this->round_deal = vector<int>{-1, -1, -1, -1};
        cfr(player_id, this->tree->getRoot(), reach_probs[1 - player_id], i, this->initial_board_long, 0);
    }
    if ((i % this->print_interval == 0 && i != 0 && i >= this->warmup) || this->nowstop) {
        float expliotibility =
            br.printExploitability(tree->getRoot(), i + 1, tree->getRoot()->getPot(), initial_board_long);
        if (expliotibility <= this->accuracy)
            break;
        if (this->nowstop) {
            this->nowstop = false;
            break;
        }
    }
}`
  },
  {
    id: "dispatch",
    title: "The walk picks a function per node",
    pipe: "CFR",
    text: "cfr is the recursive walk. An action node computes a strategy and regrets. A chance node deals the next card. A showdown node ranks both players' hands. A terminal node pays the fold amount. The returned vector has one expected value per hand in the traverser's range.",
    callout: () => "deal is 0 on the root street, then encodes the turn card, then the turn-and-river pair. That index selects which trainable slot to update.",
    file: "src/solver/PCfrSolver.cpp",
    where: "PCfrSolver::cfr",
    code: `vector<float> PCfrSolver::cfr(int player, shared_ptr<GameTreeNode> node, const vector<float>& reach_probs,
                              int iter, uint64_t current_board, int deal) {
    switch (node->getType()) {
        case GameTreeNode::ACTION:
            return actionUtility(player, action_node, reach_probs, iter, current_board, deal);
        case GameTreeNode::SHOWDOWN:
            return showdownUtility(player, showdown_node, reach_probs, iter, current_board, deal);
        case GameTreeNode::TERMINAL:
            return terminalUtility(player, terminal_node, reach_probs, iter, current_board, deal);
        case GameTreeNode::CHANCE:
            return chanceUtility(player, chance_node, reach_probs, iter, current_board, deal);
    }
}`
  },
  {
    id: "action",
    title: "Regret at a decision",
    pipe: "CFR",
    text: "The current strategy is regret matching: for each hand, an action's probability is its positive regret divided by the sum of positive regrets. The node player mixes the child values by that strategy. The opponent passes reach multiplied by the strategy into each child. Regret for an action is that action's value minus the mixed value. A positive regret means the action was better than what the hand actually played.",
    callout: () => "Example: check is worth 1 and bet is worth 3, each played 50%. The mix is worth 2. Check regret is -1. Bet regret is +1. Next iteration bets more.",
    file: "src/solver/PCfrSolver.cpp",
    where: "PCfrSolver::actionUtility",
    code: `const vector<float> current_strategy = trainable->getcurrentStrategy();

if (node_player != player) {
    // Opponent: scale reach by the probability of this action.
    new_reach_prob[hand_id] = reach_probs[hand_id] * strategy_prob;
    results[action_id] = this->cfr(player, children[action_id], new_reach_prob, iter, current_board, deal);
} else {
    results[action_id] = this->cfr(player, children[action_id], reach_probs, iter, current_board, deal);
}

// Node player: value of the mix, then regret of each action.
payoffs[hand_id] += strategy_prob * action_utilities[hand_id];
regrets[action_id * node_player_private_cards.size() + i] =
    all_action_utility[action_id][i] - payoffs[i];

if (iter > this->warmup)
    trainable->updateRegrets(regrets, iter + 1, reach_probs);`
  },
  {
    id: "dcfr",
    title: "Discount old regrets",
    pipe: "CFR",
    text: "This is Discounted CFR, not vanilla CFR. Positive regrets are multiplied by iteration^1.5 / (1 + iteration^1.5), so early noise fades. Negative regrets are multiplied by 0.5. The average strategy, which is what the GUI displays, adds the current strategy with weight (t / (t+1))^2 and then multiplies the old average by 0.9.",
    callout: () => "alpha = 1.5, beta = 0.5, gamma = 2, theta = 0.9.",
    file: "src/trainable/DiscountedCfrTrainable.cpp",
    where: "getcurrentStrategyNoCache and updateRegrets",
    code: `// Strategy from positive regrets. Equal mix when every regret is <= 0.
current_strategy[index] = max(0.0f, this->r_plus[index]) / this->r_plus_sum[private_id];

auto alpha_coef = pow(iteration_number, this->alpha);
alpha_coef = alpha_coef / (1 + alpha_coef);
this->r_plus[index] = one_reg + this->r_plus[index];
if (this->r_plus[index] > 0)
    this->r_plus[index] *= alpha_coef;
else
    this->r_plus[index] *= beta;

float strategy_coef = pow(((float)iteration_number / (iteration_number + 1)), gamma);
this->cum_r_plus[index] *= this->theta;
this->cum_r_plus[index] += current_strategy[index] * strategy_coef;`
  },
  {
    id: "chance",
    title: "Deal the next street",
    pipe: "Chance",
    text: "chanceUtility tries every card still in the deck. A card on the board is skipped. A card in an opponent hand zeros that hand's reach. Each surviving card is recursed into, and the utilities are summed. possible_deals divides the reach so the values stay an expectation over the remaining cards. OpenMP runs those deals in parallel.",
    callout: (g) => `The loop iterates node->getCards(), the ${g.deckSize}-card deck. On a flop board, a turn deal has 52 - 3 - 2 = 47 possible cards.`,
    file: "src/solver/PCfrSolver.cpp",
    where: "PCfrSolver::chanceUtility",
    code: `int possible_deals = node->getCards().size() - Card::long2board(current_board).size() - 2;

if (Card::boardsHasIntercept(card_long, current_board))
    continue;
if (this->color_iso_offset[deal][one_card->getCardInt() % 4] < 0)
    continue;

new_reach_probs[player_hand] = reach_probs[player_hand] / possible_deals;
uint64_t new_board_long = current_board | card_long;
vector<float> child_utility = this->cfr(player, one_child, new_reach_probs, iter, new_board_long, new_deal);

for (std::size_t i = 0; i < child_utility.size(); i++)
    chance_utility[i] += child_utility[i];`
  },
  {
    id: "iso",
    title: "Skip identical suits",
    pipe: "Chance",
    text: "When use_isomorphism is on, findGameSpecificIsomorphisms marks suits that look the same on the current board. chanceUtility skips a suit whose offset is negative and reuses the result of the matching suit, swapping colors in the utility vector. That is why a monotone board solves faster: several suits are the same situation.",
    callout: () => "Example: on a Qs Jh 2h flop, clubs and diamonds have no board cards, so a club turn and the matching diamond turn are solved once.",
    file: "src/solver/PCfrSolver.cpp",
    where: "PCfrSolver::findGameSpecificIsomorphisms",
    code: `for (int i = 0; i < 4; i++) {
    this->color_iso_offset[0][i] = 0;
    for (int j = 0; j < i; j++) {
        if (color_hash[i] == color_hash[j])
            this->color_iso_offset[0][i] = j - i;
    }
}

// In chanceUtility, a negative offset means "copy the other suit":
int offset = this->color_iso_offset[deal][one_card->getCardInt() % 4];
if (offset < 0) {
    child_utility = results[one_card->getNumberInDeckInt() + offset];
    exchange_color(child_utility, this->pcm.getPreflopCards(player), rank1, rank2);
}`
  },
  {
    id: "showdown",
    title: "Pay the winner at showdown",
    pipe: "Showdown",
    text: "RiverRangeManager ranks both ranges on this exact board. The comparer supplies the rank. showdownUtility then walks the two sorted lists once. Hands you beat contribute the win payoff. Hands that beat you contribute the lose payoff. Card sums remove opponents who share a hole card, so those matchups are not counted twice and are not impossible.",
    callout: (g) => `The sort order comes from ${g.file}. Smaller rank integer wins.`,
    file: "src/solver/PCfrSolver.cpp",
    where: "PCfrSolver::showdownUtility",
    code: `const vector<RiverCombs>& player_combs = this->rrm.getRiverCombos(player, player_private_cards, current_board);
const vector<RiverCombs>& oppo_combs = this->rrm.getRiverCombos(oppo, oppo_private_cards, current_board);

while (j < oppo_combs.size() && one_player_comb.rank < oppo_combs[j].rank) {
    const RiverCombs& one_oppo_comb = oppo_combs[j];
    winsum += reach_probs[one_oppo_comb.reach_prob_index];
    card_winsum[one_oppo_comb.private_cards.card1] += reach_probs[one_oppo_comb.reach_prob_index];
    card_winsum[one_oppo_comb.private_cards.card2] += reach_probs[one_oppo_comb.reach_prob_index];
    j++;
}
payoffs[one_player_comb.reach_prob_index] =
    (winsum - card_winsum[one_player_comb.private_cards.card1]
            - card_winsum[one_player_comb.private_cards.card2]) * win_payoff;`
  },
  {
    id: "fold",
    title: "Pay a fold without ranking hands",
    pipe: "Showdown",
    text: "A terminal node already knows the chip result. The fold payoff is multiplied by the opponent's total reach, minus reach on the two cards this hand uses. No comparer call happens. That is why a fold on the flop is cheap and a showdown on the river is not.",
    callout: () => "player_payoff is positive for the player who won the pot and negative for the player who folded.",
    file: "src/solver/PCfrSolver.cpp",
    where: "PCfrSolver::terminalUtility",
    code: `float player_payoff = node->get_payoffs()[player];
payoffs[i] = player_payoff * (oppo_sum - oppo_card_sum[one_player_hand.card1]
                            - oppo_card_sum[one_player_hand.card2] + plus_reach_prob);`
  },
  {
    id: "stop",
    title: "Stop when the strategy is close enough",
    pipe: "Result",
    text: "BestResponse plays a perfect counter-strategy against the current average and reports how many chips it can win. That number is exploitability. The loop ends when it drops to the accuracy target, when iterations run out, or when stop() sets nowstop. After training, one extra cfr pass with collecting_statics fills the EV tables the GUI shows.",
    callout: () => "stop() only sets a flag. The iteration finishes its current tree walk, then breaks.",
    file: "src/solver/PCfrSolver.cpp",
    where: "PCfrSolver::stop and the end of train",
    code: `void PCfrSolver::stop() {
    this->nowstop = true;
}

qDebug().noquote() << QObject::tr("collecting statics");
this->collecting_statics = true;
for (int player_id = 0; player_id < this->player_number; player_id++) {
    cfr(player_id, this->tree->getRoot(), reach_probs[1 - player_id],
        this->iteration_number, this->initial_board_long, 0);
}`
  },
  {
    id: "result",
    title: "The GUI reads the average, not the last mix",
    pipe: "Result",
    text: "get_strategy returns the cumulative average stored in cum_r_plus. That is the solved strategy. getcurrentStrategy is only the mix used inside the current iteration. get_evs returns the values stored on the last statistics pass. LocalBackend then multiplies ranges by the actions taken on the path to the clicked node.",
    callout: () => "The final iteration's mix can still swing. The average is what converges toward equilibrium.",
    file: "src/trainable/DiscountedCfrTrainable.cpp",
    where: "dump_strategy uses getAverageStrategy",
    code: `const vector<float>& average_strategy = this->getAverageStrategy();
for (int j = 0; j < this->action_number; j++) {
    std::size_t strategy_index = j * this->privateCards->size() + i;
    one_strategy[j] = average_strategy[strategy_index];
}
strategy[one_private_card.toString()] = one_strategy;`
  }
];

const pipes = ["Deck", "Tree", "Ranges", "CFR", "Chance", "Showdown", "Result"];
let cursor = 0;
let timer = null;

const nav = document.getElementById("nav");
const pipeEl = document.getElementById("pipe");
pipes.forEach(name => {
  const s = document.createElement("span");
  s.textContent = name;
  s.dataset.pipe = name;
  pipeEl.appendChild(s);
});
steps.forEach((step, i) => {
  const b = document.createElement("button");
  b.className = "step-btn";
  b.innerHTML = `${step.title}<small>${step.pipe}</small>`;
  b.onclick = () => { pause(); show(i); };
  nav.appendChild(b);
});


function show(i) {
  cursor = i;
  const step = steps[i];
  [...nav.children].forEach((b, k) => b.classList.toggle("active", k === i));
  document.getElementById("kicker").textContent = `Step ${i + 1} of ${steps.length}`;
  document.getElementById("title").textContent = step.title;
  document.getElementById("text").textContent = step.text;
  document.getElementById("callout").textContent = step.callout(game);
  document.getElementById("file").textContent = step.file;
  document.getElementById("where").textContent = step.where;
  document.getElementById("code").textContent = step.code;
  [...pipeEl.children].forEach(s => s.classList.toggle("on", s.dataset.pipe === step.pipe));
}

function play() {
  pause();
  timer = setInterval(() => {
    show(cursor >= steps.length - 1 ? 0 : cursor + 1);
  }, Number(document.getElementById("speed").value));
}
function pause() { clearInterval(timer); timer = null; }

document.getElementById("play").onclick = play;
document.getElementById("pause").onclick = pause;
document.getElementById("next").onclick = () => { pause(); show(Math.min(cursor + 1, steps.length - 1)); };
document.getElementById("prev").onclick = () => { pause(); show(Math.max(cursor - 1, 0)); };

show(0);
