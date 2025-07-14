/*
  silver rank: 274/2153
  algo: beam search with some conditions
  starting few turns (6) -> beam search with only learn moves
  endgame eval change
  global brew count bug fix
  learn count added for opponent also
  beam search width, depth and time hit and trial for these values
*/
#include <iostream>
#include <vector>
#include <cstring>
#include <unordered_map>
#include <array>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <cassert> 
#include <atomic>
#include <thread>
#include <queue>

using namespace std;


constexpr int ME = 0;
constexpr int OPP = 1;

constexpr int BREW = 0;
constexpr int CAST = 1;
constexpr int REST = 2;
constexpr int WAIT = 3;
constexpr int LEARN = 4;
const string BREW_STRING = "BREW";
const string CAST_STRING = "CAST";
const string REST_STRING = "REST";
const string WAIT_STRING = "WAIT";
const string LEARN_STRING = "LEARN";

const string MOVE_NAME[5] = {"BREW", "CAST", "REST", "WAIT", "LEARN"};

constexpr int MAX_CAPICITY = 10;
constexpr int MAX_MOVES = 30;

static constexpr int MAX_DEPTH = 20;
static double discount[MAX_DEPTH + 1]; // depth paitience score 

void initDiscount() {
    discount[0] = 1.0;
    for (int d = 1; d <= MAX_DEPTH; ++d) {
        discount[d] = discount[d-1] * 0.95;
    }
}

int game_turn = 0;
int global_brew_count[2] = {0, 0};
int global_learn_count[2] = {0, 0}; // ignoring OPP for now

struct Inv {
    // 8 bits for each type
    uint32_t p = 0;

    constexpr Inv() : p(0) {}

    // Create a new Inv from a sub-calculated 32-bit value
    // simillar to (inv[i] + 64) % 64 for each 4 invs
    // -ve -> stored as 64 - x
    // +ve -> stored as x
    explicit constexpr Inv(uint32_t x) : p((x + 0x40404040) & 0x3F3F3F3F) {}

    constexpr Inv(int a, int b, int c, int d)
      : p(
          (static_cast<uint32_t>(0x100 + a) % 0x40)        |
         ((static_cast<uint32_t>(0x100 + b) % 0x40) <<  8) |
         ((static_cast<uint32_t>(0x100 + c) % 0x40) << 16) |
         ((static_cast<uint32_t>(0x100 + d) % 0x40) << 24)
        )
    {}

    // only used this for log output to display inventory counts
    inline int operator [](int i) const {
        int x = ((p >> (i*8)) % 0x40) ;
        return (x > 0x30) ? x - 0x40 : x;
    }

    // Test for valid positive numbers (inv counts)
    inline operator bool() const {
        if ((p & 0x30303030) ||                          // Negative is invalid
            (p + 0x05050505) & 0x10101010) return false; // Positive overflow (any item > 10)

        return total() <= 10;
    }

    inline int total() const {
        uint32_t x = p + (p >> 16);
        x += x >> 8;
        return x & 0xFFu;
    }

    inline Inv operator+(Inv const & o) { return Inv{ p + o.p }; }
    inline Inv operator-(Inv const & o) { return Inv{ p - o.p }; }
    inline bool operator >=(Inv const & rhs) const { return !((p - rhs.p) & 0x30303030); } // Negative is invalid
    inline bool operator >(Inv const & rhs) const { return p != rhs.p && *this >= rhs; }
};

class Action {
public:
    int id;             // the unique ID of this spell or recipe
    // string type;        // CAST, OPPONENT_CAST, LEARN, BREW
    int delta[4];       // ingredient changes
    int price;          // the price in rupees if this is a potion
    int tome_index;     // in the first two leagues: always 0; later: the index in the tome if this is a tome spell, equal to the read-ahead tax; For brews, this is the value of the current urgency bonus
    int tax_count;      // in the first two leagues: always 0; later: the amount of taxed tier-0 ingredients you gain from learning this spell; For brews, this is how many times you can still gain an urgency bonus
    bool castable;      // in the first league: always 0; later: 1 if this is a castable player spell
    bool repeatable;    // for the first two leagues: always 0; later: 1 if this is a repeatable player spell
    
    bool completed = false; // already completed this order

    string read() {
        string type;
        std::cin >> id >> type >> delta[0] >> delta[1] >> delta[2] >> delta[3];
        std::cin >> price;
        std::cin >> tome_index >> tax_count;
        std::cin >> castable >> repeatable;
        std::cin.ignore();
        return type;
    }
};

class Player {
public:
    int id;
    int inv[4];
    int score;

    int brewed_count = 0;
    int learn_count = 0;

    Player (int id) : id(id) {}

    void read() {
        std::cin >> inv[0] >> inv[1] >> inv[2] >> inv[3] >> score; std::cin.ignore();
    }

    bool can_brew(const Action &a) { // condition (a.completed == false) is already tasted via order_mask
        return inv[0] >= -a.delta[0] && inv[1] >= -a.delta[1] && inv[2] >= -a.delta[2] && inv[3] >= -a.delta[3];
    }

    bool can_cast(const Action &a, const int t = 1) { // condition (a.castable == true) is already tasted via spell_mask
        return inv[0] >= -a.delta[0]*t && inv[1] >= -a.delta[1]*t && inv[2] >= -a.delta[2]*t && inv[3] >= -a.delta[3]*t &&
            inv[0] + inv[1] + inv[2] + inv[3] + a.delta[0]*t + a.delta[1]*t + a.delta[2]*t + a.delta[3]*t <= MAX_CAPICITY;
    }
};


class Move {
public:
    // 5 types: BREW | CAST | REST | WAIT | LEARN -> 3 bits
    // times -> can be up to 10 -> 4 bits
    // rest bits for idx -> rest all bits
    int data;
    int tax = 0; // tax if learn type, index in queue if order

    Move() {}
    Move (int type) : data(type) {}
    Move (int type, int idx) : data(type | (idx << 7)) {}
    Move (int type, int idx, int times) : data(type | (times << 3) | (idx << 7)) {}
    Move (int type, int idx, int times, int _tax) : data(type | (times << 3) | (idx << 7)), tax(_tax) {}

    int get_type() const { return data & 0b111; }
    int get_times() const { return (data >> 3) & 0b1111; }
    int get_index() const { return data >> 7; }
} WAIT_MOVE(WAIT), REST_MOVE(REST);


struct MoveList {
    Move buffer[MAX_MOVES];
    size_t sz = 0;

    Move* begin() { return buffer; }
    Move* end() { return buffer + sz; }
    // const Move* begin() const { return buffer; }
    // const Move* end() const { return buffer + sz; }

    size_t size() const { return sz; }
    void push_back(Move move) {
        assert(sz < MAX_MOVES - 1);
        buffer[sz++] = move;
    }
    Move& operator[](int i) { return buffer[i]; }
    // const Move& operator[](int i) const { return buffer[i]; }
};



struct Undo {
    int type;   // move type
    int idx;    // action index
    
    int  inv_old[4];    // player old inv
    int  score_old;     // player old score

    int bonus1_old, bonus3_old;     // game old bonuses
    
    uint64_t exhausted_mask_old;    // for REST, for indexes of exahusted spells
};

class Game {
public:
    std::array<Player, 2> p;
    std::vector<Action> orders;
    std::vector<Action> tome_spells;
    std::array<std::vector<Action>, 2> spells;
    int bonus1_left = 4;
    int bonus3_left = 4;
    int turn = game_turn;
    uint64_t orders_mask;       // bit 1 if order not completed
    uint64_t spells_mask[2];    // bit 1 if spell castable
    uint64_t tome_spells_mask;  // bit 1 if tome spell not completed
    int exhausted_spells[2];    // count of exhausted spells 

    Game () : p({Player(ME), Player(OPP)}) {}

    void sort_orders_and_spells_beforehand(std::vector<Action> &o, std::array<std::vector<Action>, 2> &sp) {
        // high price (including ingridient price) high priority for both brew and cast
        sort(o.begin(), o.end(), [&](const Action &a1, const Action &a2) {
            return a1.price + a1.delta[1] + a1.delta[2] + a1.delta[3]
                > a2.price + a2.delta[1] + a2.delta[2] + a2.delta[3];
        });
        for (auto &vec : sp) {
            sort(vec.begin(), vec.end(), [&](const Action &a1, const Action &a2) {
                return a1.delta[1] + a1.delta[2] + a1.delta[3]
                    > a2.delta[1] + a2.delta[2] + a2.delta[3];
            });
        }
    }

    void set_items(std::vector<Action> &o, std::array<std::vector<Action>, 2> &sp, std::vector<Action> &t_spells) {
        reset();
        // sort_orders_and_spells_beforehand(o, sp);
        for (int i = 0; i < o.size(); i++) {
            orders.emplace_back(o[i]);
            if (!o[i].completed) {
                assert(i < 64);
                orders_mask |= (uint64_t(1) << i);
            }
        }
        for (int pid = 0; pid < 2; pid++) {
            auto &vec = sp[pid];
            for (int i = 0; i < vec.size(); i++) {
                spells[pid].emplace_back(vec[i]);
                if (vec[i].castable) {
                    assert(i < 64);
                    spells_mask[pid] |= (uint64_t(1) << i);
                } else {
                    exhausted_spells[pid]++;
                }
            }
        }
        for (int i = 0; i < t_spells.size(); i++) {
            tome_spells.emplace_back(t_spells[i]);
            if (!t_spells[i].completed) {
                assert(i < 64);
                tome_spells_mask |= (uint64_t(1) << i);
            }
        }
    }

    void reset() {
        orders.clear();
        spells[ME].clear();
        spells[OPP].clear();
        tome_spells.clear();
        turn = game_turn;
        p[ME].brewed_count = global_brew_count[ME];
        p[OPP].brewed_count = global_brew_count[OPP];
        p[ME].learn_count = global_learn_count[ME];
        p[OPP].learn_count = global_learn_count[OPP];
        orders_mask = spells_mask[ME] = spells_mask[OPP] = tome_spells_mask = uint64_t(0);
        exhausted_spells[ME] = exhausted_spells[OPP] = 0;
    }

    MoveList get_all_moves(const int &pid) {
        MoveList moves;
        uint64_t mask = orders_mask;
        int index_in_queue = 0;
        while (mask) {
            int idx = __builtin_ctzll(mask);   // extract lowest set bit index
            if (p[pid].can_brew(orders[idx])) {
                moves.push_back({BREW, idx, 0, index_in_queue});
            }
            mask &= mask - 1;   // clear lowest set bit
            index_in_queue++;
        }

        mask = spells_mask[pid];
        while (mask) {
            int idx = __builtin_ctzll(mask);
            Action &a = spells[pid][idx];
            if (p[pid].can_cast(a)) {
                moves.push_back({CAST, idx});
                if (a.repeatable) {
                    for (int times = 2; p[pid].can_cast(a, times); times++) {
                        moves.push_back({CAST, idx, times});
                    }
                }
            }
            mask &= mask - 1;
        }

        mask = tome_spells_mask;
        int tax_to_pay = 0;
        while (mask && p[pid].inv[0] >= tax_to_pay) {
            int idx = __builtin_ctzll(mask);
            moves.push_back({LEARN, idx, 0, tax_to_pay});
            mask &= mask - 1;
            tax_to_pay++;
        }

        if (exhausted_spells[pid] > 0) {
            moves.push_back(REST_MOVE);
        }
        if (moves.size() == 0) {
            moves.push_back(WAIT_MOVE);
        }
        return moves;
    }

    // calcs heu. score of a move
    double get_heurestinc_score(const Move &move, const int pid) {
        static constexpr int ING_W[4] = {1, 5, 25, 125};
        
        // brew > cast > learn > rest > wait
        //                  indices order -> brew, cast, rest, wait, learn
        static constexpr double TYPE_P[5] = {4000, 3000, 1000, 0, 2000};
        
        // tuning factors
        static constexpr double PRICE_FACTOR        = 50.0;
        static constexpr double COST_FACTOR         = 1.0;
        static constexpr double QUEUE_PENALTY       = 1.0;
        static constexpr double CAST_GAIN_FACTOR    = 5.0;
        static constexpr double CAST_SPENT_PENALTY  = 1.0;
        static constexpr double LEARN_GAIN_FACTOR   = 5.0;
        static constexpr double LEARN_SPENT_PENALTY = 1.0;
        static constexpr double LEARN_COST_PENALTY  = 20.0;

        int type = move.get_type();
        int idx = move.get_index();
        double sc = TYPE_P[type];

        if (type == BREW) {
            auto &a = orders[idx];
            double reward = 0;
            for (int t = 0; t < 4; ++t) {
                reward += a.delta[t] * ING_W[t];
            }
            sc += a.price * PRICE_FACTOR
                + reward * COST_FACTOR
                - move.tax * QUEUE_PENALTY;
        }
        else if (type == CAST) {
            auto &a = spells[pid][idx];
            double gain = 0, spent = 0;
            for (int t = 0; t < 4; ++t) {
                int d = a.delta[t] * max((int)move.get_times(), 1);
                if (d > 0) gain += d * ING_W[t];
                else spent += -d * ING_W[t];
            }
            sc += gain * CAST_GAIN_FACTOR
                - spent * CAST_SPENT_PENALTY;
        }
        else if (type == LEARN) {
            auto &a = tome_spells[idx];
            // you net gain 'tax_count' tier‑0 tokens next turn
            // plus any free‐ingredient spells (sum of positive deltas)
            double gain = a.tax_count * ING_W[0], spent = 0;
            for (int t = 0; t < 4; ++t) {
                if (a.delta[t] > 0) gain += a.delta[t] * ING_W[t];
                else spent += -a.delta[t] * ING_W[t];
            }
            sc += gain * LEARN_GAIN_FACTOR
                - spent * LEARN_SPENT_PENALTY
                - a.tome_index * LEARN_COST_PENALTY;
        }
        return sc;
    }

    void sort_moves(MoveList &moves, const int pid) {
        double h_score[moves.size()];
        int move_index_to_move_arr_idx[64];
        for (int i = 0; i < moves.size(); i++) {
            h_score[i] = get_heurestinc_score(moves[i], pid);
            move_index_to_move_arr_idx[moves[i].get_index()] = i;
        }
        sort(moves.begin(), moves.end(), [&](const Move &a, const Move &b) {
            return h_score[move_index_to_move_arr_idx[a.get_index()]] 
                > h_score[move_index_to_move_arr_idx[b.get_index()]];
        });
    }

    void brew(const Move &move, const int &pid) {
        int idx = move.get_index();
        Action &a = orders[idx];
        p[pid].inv[0] += a.delta[0];
        p[pid].inv[1] += a.delta[1];
        p[pid].inv[2] += a.delta[2];
        p[pid].inv[3] += a.delta[3];
        p[pid].score += a.price;
        p[pid].brewed_count++;
        orders_mask ^= (uint64_t(1) << idx);

        int index_in_queue = move.tax;
        if (index_in_queue == 0) {
            if (bonus3_left > 0) {
                p[pid].score += 3;
                bonus3_left--;
            } else if (bonus1_left > 0) {
                p[pid].score += 1;
                bonus1_left--;
            }
        } else if (index_in_queue == 1 && bonus3_left > 0 && bonus1_left > 0) {
            p[pid].score += 1;
            bonus1_left--;
        }
    }

    // avoid putting tax as not simulation OPP, but need to substract need to pay tax for further correctness of ingredients
    void learn(const Move &move, const int &pid) {
        int index = move.get_index(), tax_to_pay = move.tax;
        Action &a = tome_spells[index];

        spells[pid].emplace_back(a);
        tome_spells_mask ^= (uint64_t(1) << index);
        p[pid].inv[0] -= tax_to_pay;

        int inv_space = MAX_CAPICITY - (p[pid].inv[0] + p[pid].inv[1] + p[pid].inv[2] + p[pid].inv[3]);
        int tax_available = a.tax_count;

        p[pid].inv[0] += min(inv_space, tax_available); // take the tax, discard overflow
        p[pid].learn_count++;
    }

    void cast(const Move &move, const int &pid) {
        int idx = move.get_index();
        Action &a = spells[pid][idx];
        int t = move.get_times() == 0 ? 1 : move.get_times();
        p[pid].inv[0] += a.delta[0] * t;
        p[pid].inv[1] += a.delta[1] * t;
        p[pid].inv[2] += a.delta[2] * t;
        p[pid].inv[3] += a.delta[3] * t;
        spells_mask[pid] ^= (uint64_t(1) << idx);
        exhausted_spells[pid]++;
    }

    uint64_t rest(const Move &move, const int &pid) {
        uint64_t mask = spells_mask[pid];
        spells_mask[pid] = (uint64_t(1) << (spells[pid].size())) - 1;
        return mask;
    }

    Undo make_move(Move &move, const int &pid) {
        auto move_type = move.get_type();
        auto index = move.get_index();

        Undo undo;
        undo.idx = move.get_index();
        for (int i = 0; i < 4; i++) {
            undo.inv_old[i] = p[pid].inv[i];
        }
        undo.score_old = p[pid].score;
        undo.bonus1_old = bonus1_left;
        undo.bonus3_old = bonus3_left;

        if (move_type == REST) {
            undo.type = REST;
            undo.exhausted_mask_old = rest(move, pid);
        } else if (move_type == BREW) {
            undo.type = BREW;
            brew(move, pid);
        } else if (move_type == CAST) {
            undo.type = CAST;
            cast(move, pid);
        } else if (move_type == LEARN) {
            undo.type = LEARN;
            learn(move, pid);
        } else {
            undo.type = WAIT;
        }

        turn++;
        return undo;
    }

    void undo_move(const Undo &undo, const int &pid) {
        for (int i = 0; i < 4; i++) {
            p[pid].inv[i] = undo.inv_old[i];
        }
        p[pid].score = undo.score_old;
        bonus1_left = undo.bonus1_old;
        bonus3_left = undo.bonus3_old;

        if (undo.type == BREW) {
            p[pid].brewed_count--;
            orders_mask ^= (uint64_t(1) << undo.idx);
        } else if (undo.type == LEARN) {
            p[pid].learn_count--;
            spells[pid].pop_back();
            tome_spells_mask ^= (uint64_t(1) << undo.idx);
        } else if (undo.type == CAST) {
            exhausted_spells[pid]--;
            spells_mask[pid] ^= (uint64_t(1) << undo.idx);
        } else if (undo.type == REST) {
            spells_mask[pid] = undo.exhausted_mask_old;
        }
        turn--;
    }

    double get_simple_score(const int pid = ME) {
        double eval = p[pid].score;
        for (int i = 1; i < 4; i++) {
            eval += p[pid].inv[i];
        }
        return eval;
    }

    double get_eval(const int depth, const int pid = ME) {
        double eval = 0.0;
        for (int i = 1; i <= 4; i++) {
            eval += i * p[pid].inv[i-1];
        }
        eval += p[pid].score;
        eval += 1.1 * p[pid].brewed_count;
        eval += 0.5 * p[pid].learn_count;

        if (p[pid].brewed_count == 6 || p[1-pid].brewed_count == 6) {
            double e1 = get_simple_score(ME), e2 = get_simple_score(OPP);
            if (e1 > e2) return 1e9 + e1 - e2;
            else if (e1 < e2) return -1e9 + e1 - e2;
            return 0;
        }

        if (p[pid].brewed_count == 5 || p[1-pid].brewed_count == 5) {            
            eval *= depth < MAX_DEPTH ? discount[depth] * discount[depth] : 1.0;
        } else {
            eval *= depth < MAX_DEPTH ? discount[depth] : 1.0;
        }
        // if (p[pid].brewed_count - global_brew_count[pid] > 0) {
        //     eval += 20;
        // }
        return eval;
    }

    double get_eval_both(const int depth) {
        return get_eval(depth, ME) - get_eval(depth, OPP);
    }

    int get_id_from_index(const int &type, const int &idx) {
        switch (type) {
            case BREW: return orders[idx].id;
            case CAST: return spells[ME][idx].id;
            case LEARN: return tome_spells[idx].id;
            default: return 0;
        }
    }

    void output_move(const Move &move) {
        int type = move.get_type(), idx = move.get_index(), times = move.get_times();
        int id = get_id_from_index(type, idx);
        std::cout << MOVE_NAME[type];
        if (type == BREW || type == CAST || type == LEARN) {
            std::cout << " " << id;
            if (type == CAST && times > 1) {
                std::cout << " " << times;
            }
        }
        std::cout << std::endl;
    }
};

std::atomic<bool> time_up{false};
void start_timer(int ms) {
    std::thread([ms](){
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        time_up = true;
    }).detach();
}
int LEARN_MOVE_FORCE_TURNS = 6;
class Iterative_deepening {
    int max_moves_found = 0;
    int nodes_visited = 0;

    std::pair<double, Move> find_best_move_time_controlled(Game &game, const int depth, const int &max_depth, const Move last_move = WAIT_MOVE) {
        nodes_visited++;
        if (time_up || depth >= max_depth || 
            // last_move.get_type() == BREW || 
            game.p[ME].brewed_count >= 6 || game.p[OPP].brewed_count >= 6 || game.turn >= 100) {
            return {game.get_eval(depth), WAIT_MOVE};
        }
        double best_eval = -1e9;
        Move best_move = WAIT_MOVE;

        MoveList moves = game.get_all_moves(ME);
        max_moves_found = max(max_moves_found, (int)moves.size());
        // if avalable move is only WAIT then stop further searching
        if (moves.size() == 1 && moves[0].get_type() == WAIT) return {game.get_eval(depth), moves[0]};

        for (int i = 0; i < moves.size(); i++) {
            if (time_up) return {best_eval, best_move};
            Move move = moves[i];
            if (game.turn <= LEARN_MOVE_FORCE_TURNS && move.get_type() != LEARN) continue; // force starting 6 turn moves to be LEARN
            Undo undo = game.make_move(move, ME);
            auto [eval, temp_move] = find_best_move_time_controlled(game, depth + 1, max_depth, move);
            game.undo_move(undo, ME);
            if (eval > best_eval) {
                best_eval = eval;
                best_move = move;
            }
        }
        return {best_eval, best_move};
    }

    // consider both player's moves
    std::pair<double, Move> find_best_move_time_controlled2(Game &game, const int depth, const int &max_depth, const Move last_move = WAIT_MOVE) {
        nodes_visited++;
        if (time_up || depth >= max_depth || 
            // last_move.get_type() == BREW || 
            game.p[ME].brewed_count >= 6 || game.p[OPP].brewed_count >= 6 || game.turn >= 100) {
            return {game.get_eval_both(depth), WAIT_MOVE};
        }
        double best_eval = game.get_eval_both(depth);
        Move best_move = WAIT_MOVE;

        MoveList my_moves = game.get_all_moves(ME);
        max_moves_found = max(max_moves_found, (int)my_moves.size());
        // if avalable move is only WAIT then stop further searching
        if (my_moves.size() == 1 && my_moves[0].get_type() == WAIT) return {game.get_eval_both(depth), my_moves[0]};
        
        for (int i = 0; i < my_moves.size(); i++) {
            if (time_up) return {best_eval, best_move};
            Move my_move = my_moves[i];
            Undo undo1 = game.make_move(my_move, ME);

            MoveList opp_moves = game.get_all_moves(OPP);
            for (int j = 0; j < opp_moves.size(); j++) {
                if (time_up) return {best_eval, best_move};
                Move opp_move = opp_moves[j];
                Undo undo2 = game.make_move(opp_move, OPP);
                auto [eval, temp_move] = find_best_move_time_controlled2(game, depth + 1, max_depth, my_move);
                game.undo_move(undo2, OPP);
                if (eval > best_eval) {
                    best_eval = eval;
                    best_move = my_move;
                }
            }
            game.undo_move(undo1, ME);
        }
        return {best_eval, best_move};
    }

public:
    Move find_best(const Game &game, const int &time_ms) {
        time_up = false;
        start_timer(time_ms);
        Move best_move = Move(WAIT);
        int max_depth = 1;
        int max_nodes_visited = 0;
        Game g = game;
        for (; !time_up; max_depth++) {
            nodes_visited = 0;
            best_move = find_best_move_time_controlled(g, 0, max_depth).second;
            // best_move = find_best_move_time_controlled2(g, 0, max_depth).second;
            max_nodes_visited = max(max_nodes_visited, nodes_visited);
        }
        cerr << "max depth reached : " << max_depth << endl;
        cerr << "max moves found : " << max_moves_found << endl;
        cerr << "max nodes visited : " << max_nodes_visited << endl;
        return best_move;
    }
};



struct Bfs_node {
    Game game;
    Move root_move;
    int depth;
};
Move simple_bfs (Game &game, const int time_ms, const int max_depth) {
    time_up = false;
    start_timer(time_ms);
    double best_eval = -1e9;
    Move best_move = WAIT_MOVE;
    queue<Bfs_node> q;
    for (Move &move : game.get_all_moves(ME)) {
        if (time_up) return best_move;
        Undo undo = game.make_move(move, ME);
        double eval = game.get_eval(1);
        q.push({game, move, 1});
        game.undo_move(undo, ME);
        if (eval > best_eval) {
            best_eval = eval;
            best_move = move;
        }
    }

    int depth = 1;
    while (!time_up && !q.empty() && depth < max_depth) {
        int sz = q.size();
        while (sz--) {
            auto node = q.front(); q.pop();
            if (time_up) return best_move;
            for (Move move : node.game.get_all_moves(ME)) {
                if (time_up) return best_move;
                Undo undo = node.game.make_move(move, ME);
                double eval = node.game.get_eval(depth + 1);
                q.push({node.game, node.root_move});
                game.undo_move(undo, ME);
                if (eval > best_eval) {
                    best_move = node.root_move;
                    best_eval = eval;
                }
            }
        }
        depth++;
    }
    return best_move;
}

struct Beam_node {
    Game state;         // the game after applying this node’s move sequence
    Move root_move;     // the very first move from the actual root
    double eval;        // evaluation of this node
};
MoveList get_top_moves(const Game &game, MoveList &moves) {
    int b_count = 0, l_count = 0, c_count = 0;
    MoveList top_moves;
    if (game.turn <= LEARN_MOVE_FORCE_TURNS) {
        for (Move &move : moves) {
            if (move.get_type() == LEARN) {
                top_moves.push_back(move);
            }
        }
        return moves;
    }
    for (Move &move : moves) {
        if (move.get_type() == BREW && b_count++ < 2) top_moves.push_back(move);
        if (move.get_type() == CAST && c_count++ < 2) top_moves.push_back(move);
        if (move.get_type() == LEARN && l_count++ < 2) top_moves.push_back(move);
    }
    return top_moves;
}
Move beam_search(Game &root, int max_depth, int beam_width, int time_ms) {
    time_up = false;
    start_timer(time_ms);
    vector<Beam_node> beam;
    auto first_moves = root.get_all_moves(ME);
    root.sort_moves(first_moves, ME);
    // first_moves = get_top_moves(first_moves);

    double best_eval = -1e9;
    Move best_move = WAIT_MOVE;
    for (int i = 0; i < first_moves.sz && i < beam_width; ++i) {
        if (time_up) return best_move;
        Undo u = root.make_move(first_moves[i], ME);
        double eval = root.get_eval(1);
        if (eval > best_eval) {
            best_move = first_moves[i];
            best_eval = eval;
        }
        beam.push_back({ root, first_moves[i], eval });
        root.undo_move(u, ME);
    }

    for (int depth = 2; depth <= max_depth; ++depth) {
        if (time_up) return best_move;
        vector<Beam_node> next_beam;
        next_beam.reserve(beam_width);

        for (auto &node : beam) {
            if (time_up) return best_move;
            auto moves = node.state.get_all_moves(ME);
            node.state.sort_moves(moves, ME);
            // moves = get_top_moves(moves);

            for (int i = 0; i < min<int>(beam_width, moves.sz); ++i) {
                if (time_up) return best_move;
                Undo u = node.state.make_move(moves[i], ME);
                double eval = node.state.get_eval(depth);
                if (eval > best_eval) {
                    best_move = node.root_move;
                    best_eval = eval;
                }
                next_beam.push_back({ node.state, node.root_move, eval });
                node.state.undo_move(u, ME);
            }
        }
        if (time_up || next_beam.empty()) return best_move;

        beam.swap(next_beam);
        if (beam.size() > beam_width) {
            sort(beam.begin(), beam.end(), [&](auto &A, auto &B){ return A.eval > B.eval; });
            beam.resize(beam_width);
        }
    }

    return best_move;
}


// void get_fastest_brew(Game &game) {
//     for (int i = 0; i < game.orders.size(); i++) {

//     }
// }


int main() {
    initDiscount();
    game_turn = 0;
    global_brew_count[ME] = global_brew_count[OPP] = 0;
    global_learn_count[ME] = global_learn_count[OPP] = 0;

    Game game;
    int prev_score[2] = {0, 0};
    int prev_spells[2] = {0, 0};

    while (1) {
        game.reset();
        game_turn++;

        int action_count; // the number of spells, tome spells and recipes in play
        std::cin >> action_count; std::cin.ignore();
        
        std::vector<Action> orders, tome_spells;
        std::array<std::vector<Action>, 2> spells;
        for (int i = 0; i < action_count; i++) {
            Action action;
            string type = action.read();
            if (type == "CAST") {
                spells[ME].emplace_back(action);
            } else if (type == "OPPONENT_CAST") {
                spells[OPP].emplace_back(action);
            } else if (type == "BREW") {
                orders.emplace_back(action);
                if (action.tome_index == 3) {
                    game.bonus3_left = action.tax_count;
                } else if (action.tome_index == 1) {
                    game.bonus1_left = action.tax_count;
                }
            } else if (type == "LEARN") {
                tome_spells.emplace_back(action);
            }
        }
        game.set_items(orders, spells, tome_spells);
        game.p[ME].read();
        game.p[OPP].read();

        if (game_turn > 1) {
            for (int pid = 0; pid <= 1; pid++) {
                if (game.p[pid].score != prev_score[pid]) {
                    game.p[pid].brewed_count++;
                    global_brew_count[pid]++;
                }
                if (game.spells[pid].size() != prev_spells[pid]) {
                    game.p[pid].learn_count++;
                    global_learn_count[pid]++;
                }
            }
        }



        LEARN_MOVE_FORCE_TURNS = 6;


        // int time_ms = 35; // max 50 ms per move
        // LEARN_MOVE_FORCE_TURNS = 6;
        // Iterative_deepening iddfs;
        // Move move = iddfs.find_best(game, time_ms);
        // game.output_move(move);

        // int time_ms = 38;
        // int max_depth = 100 - game_turn;
        // int max_width = 3;
        // Move move = bfs(game, time_ms, max_depth, max_width);
        // game.output_move(move);


        // Beam Search
        int time_ms = 20;
        int max_depth = min(100 - game_turn, 15);
        int max_width = 100;
        Move move = beam_search(game, max_depth, max_width, time_ms);
        game.output_move(move);

        for (int pid = 0; pid <= 1; pid++) {
            prev_score[pid] = game.p[pid].score;
            prev_spells[pid] = game.spells[pid].size();
        }
    }
    return 0;
}














