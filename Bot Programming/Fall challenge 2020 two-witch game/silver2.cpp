/* 
  rank 564/2152 
  Iterative deepening dfs algorithm with first 5-6 moves only learn 0
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

    bool can_brew(const Action &a) {
        return a.completed == false && inv[0] >= -a.delta[0] && inv[1] >= -a.delta[1] && inv[2] >= -a.delta[2] && inv[3] >= -a.delta[3];
    }

    bool can_cast(const Action &a, const int t = 1) {
        return a.castable && 
            inv[0] >= -a.delta[0]*t && inv[1] >= -a.delta[1]*t && inv[2] >= -a.delta[2]*t && inv[3] >= -a.delta[3]*t &&
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
};


struct MoveList {
    Move buffer[MAX_MOVES];
    size_t sz = 0;

    size_t size() const { return sz; }
    void push_back(Move move) {
        assert(sz < MAX_MOVES - 1);
        buffer[sz++] = move;
    }
    Move& operator[](int i) { return buffer[i]; }
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
        }
        for (int pid = 0; pid < 2; pid++) {
            auto &vec = sp[pid];
            for (int i = 0; i < vec.size(); i++) {
                spells[pid].emplace_back(vec[i]);
            }
        }
        for (int i = 0; i < t_spells.size(); i++) {
            tome_spells.emplace_back(t_spells[i]);
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
    }

    MoveList get_all_moves(const int &pid) {
        MoveList moves;
        int index_in_queue = 0;
        for (int i = 0; i < orders.size(); i++) {
            Action &a = orders[i];
            if (a.completed == false) {
                if (p[pid].can_brew(a)) {
                    moves.push_back({BREW, i, 0, index_in_queue});
                }
                index_in_queue++;
            }
        }
        for (int i = 0; i < spells[pid].size(); i++) {
            Action &a = spells[pid][i];
            if (p[pid].can_cast(a)) {
                moves.push_back({CAST, i});
                if (a.repeatable) {
                    for (int times = 2; p[pid].can_cast(a, times); times++) {
                        moves.push_back({CAST, i, times});
                    }
                }
            }
        }
        int tax_to_pay = 0;
        for (int i = 0; i < tome_spells.size(); i++) {
            Action &a = tome_spells[i];
            if (a.completed == false && p[pid].inv[0] >= tax_to_pay) {
                moves.push_back({LEARN, i, 0, tax_to_pay});
                tax_to_pay++;
                if (p[pid].inv[0] < tax_to_pay) {
                    break;
                }
            }
        }

        bool rest_possible = false;
        for (Action &a : spells[pid]) {
            if (a.castable == false) {
                moves.push_back({REST});
                rest_possible = true;
                break;
            }
        }
        if (!rest_possible && moves.size() == 0) {
            moves.push_back({WAIT});
        }
        return moves;
    }

    void brew(const Move &move, const int &pid) {
        Action &a = orders[move.get_index()];
        p[pid].inv[0] += a.delta[0];
        p[pid].inv[1] += a.delta[1];
        p[pid].inv[2] += a.delta[2];
        p[pid].inv[3] += a.delta[3];
        p[pid].score += a.price;
        p[pid].brewed_count++;
        a.completed = true;

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
        a.completed = true;
        p[pid].inv[0] -= tax_to_pay;

        int inv_space = MAX_CAPICITY - (p[pid].inv[0] + p[pid].inv[1] + p[pid].inv[2] + p[pid].inv[3]);
        int tax_available = a.tax_count;

        p[pid].inv[0] += min(inv_space, tax_available); // take the tax, discard overflow
        p[pid].learn_count++;
    }

    void cast(const Move &move, const int &pid) {
        Action &a = spells[pid][move.get_index()];
        int t = move.get_times() == 0 ? 1 : move.get_times();
        p[pid].inv[0] += a.delta[0] * t;
        p[pid].inv[1] += a.delta[1] * t;
        p[pid].inv[2] += a.delta[2] * t;
        p[pid].inv[3] += a.delta[3] * t;
        a.castable = false;
    }

    uint64_t rest(const Move &move, const int &pid) {
        uint64_t mask = 0;
        for (size_t i = 0; i < spells[pid].size(); i++) {
            // assert(i < 64);
            if (!spells[pid][i].castable) {
                mask |= (uint64_t(1) << i);
                spells[pid][i].castable = true;
            }
        }
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
            orders[undo.idx].completed = false;
        } else if (undo.type == LEARN) {
            p[pid].learn_count--;
            spells[pid].pop_back();
            tome_spells[undo.idx].completed = false;
        } else if (undo.type == CAST) {
            spells[pid][undo.idx].castable = true;
        } else if (undo.type == REST) {
            uint64_t mask = undo.exhausted_mask_old;
            while (mask) {
                // extract lowest set bit index
                unsigned idx = __builtin_ctzll(mask);
                spells[pid][idx].castable = false;
                // clear it
                mask &= mask - 1;
            }
        }
        turn--;
    }

    double get_eval(const int &depth) {
        double eval = 0.0;
        eval += p[ME].score;
        for (int i = 1; i <= 4; i++) {
            eval += i * p[ME].inv[i-1];
        }
        eval += 1.1 * p[ME].brewed_count;
        eval += 0.5 * p[ME].learn_count;

        assert(depth < MAX_DEPTH);
        eval *= discount[depth];
        return eval;
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

class Iterative_deepening {
    std::atomic<bool> time_up{false};

    void start_timer(int ms) {
        std::thread([ms, this](){
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
            time_up = true;
        }).detach();
    }

    int max_moves_found = 0;
    int nodes_visited = 0;

    std::pair<double, Move> find_best_move_time_controlled(Game &game, const int depth, const int &max_depth, const Move last_move = {WAIT}) {
        if (time_up) {
            return {-1e9, Move(WAIT)};
        }
        nodes_visited++;

        if (last_move.get_type() == BREW) { // targeting BREW, dont know if better, just testing
            return {game.get_eval(depth), Move(WAIT)};
        }

        if (depth == max_depth) {
            return {game.get_eval(depth), Move(REST)};
        }
        if (game.p[ME].brewed_count >= 6 || game.p[OPP].brewed_count >= 6 || game.turn >= 100) {
            return {game.get_eval(depth), Move(REST)};
        }
        double best_eval = -1e9;
        Move best_move = Move(REST);

        MoveList moves = game.get_all_moves(ME);
        max_moves_found = max(max_moves_found, (int)moves.size());
        if (moves.size() == 1 && moves[0].get_type() == WAIT) { // if avalable move is only WAIT then stop further searching
            return {game.get_eval(depth), moves[0]};
        }
        for (int i = 0; i < moves.size(); i++) {
            if (time_up) {
                return {best_eval, best_move};
            }
            Move my_move = moves[i];

            Undo undo = game.make_move(my_move, ME);
            auto [eval, temp_move] = find_best_move_time_controlled(game, depth + 1, max_depth, my_move);
            game.undo_move(undo, ME);
            
            if (eval > best_eval) {
                best_eval = eval;
                best_move = my_move;
            }
        }
        return {best_eval, best_move};
    }

public:
    Move find_best(const Game &game, const int &time_ms) {
        if (game.turn < 6) {
            Move move(LEARN, 0);
            return move;
        }
        time_up = false;
        start_timer(time_ms);
        Move best_move = Move(WAIT);
        int max_depth = 1;
        int max_nodes_visited = 0;
        Game g = game;
        for (; !time_up; max_depth++) {
            nodes_visited = 0;
            best_move = find_best_move_time_controlled(g, 0, max_depth).second;
            max_nodes_visited = max(max_nodes_visited, nodes_visited);
        }
        cerr << "max depth reached : " << max_depth << endl;
        cerr << "max moves found : " << max_moves_found << endl;
        cerr << "max nodes visited : " << max_nodes_visited << endl;
        return best_move;
    }
};


int main() {
    initDiscount();

    Game game;
    int prev_score[2] = {0, 0};

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

        for (int pid = 0; pid <= 1; pid++) {
            if (game.p[pid].score != prev_score[pid]) {
                game.p[pid].brewed_count++;
                global_brew_count[pid]++;
            }
        }

        // BREW <id> | WAIT; later: BREW <id> | CAST <id> [<times>] | LEARN <id> | REST | WAIT

        int time_ms = 35; // max 50 ms per move
        Iterative_deepening iddfs;
        Move move = iddfs.find_best(game, time_ms);
        game.output_move(move);

        if (move.get_type() == LEARN) {
            global_learn_count[ME]++;
        }
        prev_score[ME] = game.p[ME].score;
        prev_score[OPP] = game.p[OPP].score;
    }
    return 0;
}















