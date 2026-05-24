#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <memory>
#include <queue>
#include <unordered_map>

using namespace std;

const int GRASS = 1;
const int WATER = 2;
const int ROCK = 3;
const int IRON = 4;
const int MY_SHACK = 5;
const int OPP_SHACK = 6;

const int MOVE_TYPE = 0;
const int HARVEST_TYPE = 1;
const int PLANT_TYPE = 2;
const int CHOP_TYPE = 3;
const int PICK_TYPE = 4;
const int DROP_TYPE = 5;
const int TRAIN_TYPE = 6;
const int MINE_TYPE = 7;
const int WAIT_TYPE = 8;
const string ACTION_TYPE_STR[9] = {"MOVE", "HARVEST", "PLANT", "CHOP", "PICK", "DROP", "TRAIN", "MINE", "WAIT"};

const int PLUM_TYPE = 0;
const int LEMON_TYPE = 1;
const int APPLE_TYPE = 2;
const int BANANA_TYPE = 3;
const string FRUIT_STR[4] = {"PLUM", "LEMON", "APPLE", "BANANA"};

const int PLANED_TROLLS = 3;

enum class Phase { Planting, Training, Harvest, Mixed };

class Point {
public:
    int x, y;

    Point() : x(0), y(0) {}
    Point(const int& x_, const int& y_) : x(x_), y(y_) {}
    Point(const int& x_, const int& y_, int id_) : x(x_), y(y_) {}

    inline Point operator+(const Point& b) const { return Point(x + b.x, y + b.y); }

    friend int manhattan(const Point a, const Point b) { return abs(a.x - b.x) + abs(a.y - b.y); }

    vector<Point> neighbors() const {
        static const vector<Point> deltas = {Point(1, 0), Point(-1, 0), Point(0, 1), Point(0, -1)};
        vector<Point> res;
        for (const Point &d : deltas) res.push_back(*this + d);
        return res;
    }

    inline bool operator<(const Point& other) const {
        if (x != other.x) return x < other.x;
        return y < other.y;
    }
    inline bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

class Tree {
public:
    int type;
    Point pos;
    int size;
    int health;
    int fruits;
    int cooldown;

    void read() {
        string t;
        cin >> t >> pos.x >> pos.y >> size >> health >> fruits >> cooldown;
        cin.ignore();
        for (int i = 0; i < 4; i++) {
            if (FRUIT_STR[i] == t) {
                type = i;
                break;
            }
        }
    }
};

class Troll {
public:
    int id;
    int player;
    Point pos;
    int movement_speed;
    int carry_capacity;
    int harvest_power;
    int chop_power;
    int carry_plum;
    int carry_lemon;
    int carry_apple;
    int carry_banana;
    int carry_iron;
    int carry_wood;

    void read() {
        cin >> id >> player >> pos.x >> pos.y >> movement_speed >> carry_capacity >> harvest_power >> chop_power >> carry_plum >> carry_lemon >> carry_apple >> carry_banana >> carry_iron >> carry_wood;
        cin.ignore();
    }

    inline bool carry_any_fruit() const { return carry_plum + carry_lemon + carry_apple + carry_banana > 0; }
    inline bool is_full() const { return total_carry() >= carry_capacity; }
    inline int free_capacity() const { return carry_capacity - total_carry(); }
    inline int total_carry() const { return carry_plum + carry_lemon + carry_apple + carry_banana + carry_iron + carry_wood; }
};

class Inventory {
public:
    int plum = 0;
    int lemon = 0;
    int apple = 0;
    int banana = 0;
    int iron = 0;
    int wood = 0;

    void read() {
        cin >> plum >> lemon >> apple >> banana >> iron >> wood;
        cin.ignore();
    }

    inline int total_fruits() const {
        return plum + lemon + apple + banana;
    }
};

class Player {
public:
    vector<Troll> trolls;
    Inventory inv;
};

class Action {
public:
    virtual ~Action() = default;
    virtual string get_command() = 0;
};
class MoveAction : public Action {
public:
    int troll_id;
    Point pos;
    MoveAction(int id, const Point &p) : troll_id(id), pos(p) {}
    string get_command() override {
        return "MOVE " + to_string(troll_id) + " " + to_string(pos.x) + " " + to_string(pos.y);
    }
};
class HarvestAction : public Action {
public:
    int troll_id;
    HarvestAction(int id) : troll_id(id) {}
    string get_command() override {
        return "HARVEST " + to_string(troll_id);
    }
};
class PlantAction : public Action {
public:
    int troll_id, fruit_type;
    PlantAction(int id, int ft) : troll_id(id), fruit_type(ft) {}
    string get_command() override {
        return "PLANT " + to_string(troll_id) + " " + FRUIT_STR[fruit_type];
    }
};
class ChopAction : public Action {
public:
    int troll_id;
    ChopAction(int id) : troll_id(id) {}
    string get_command() override {
        return "CHOP " + to_string(troll_id);
    }
};
class PickAction : public Action {
public:
    int troll_id, fruit_type;
    PickAction(int id, int ft) : troll_id(id), fruit_type(ft) {}
    string get_command() override {
        return "PICK " + to_string(troll_id) + " " + FRUIT_STR[fruit_type];
    }
};
class DropAction : public Action {
public:
    int troll_id;
    DropAction(int id) : troll_id(id) {}
    string get_command() override {
        return "DROP " + to_string(troll_id);
    }
};
class TrainAction : public Action {
public:
    int movement_speed, carry_capacity, harvest_power, chop_power;
    TrainAction(int a, int b, int c, int d) : movement_speed(a), carry_capacity(b), harvest_power(c), chop_power(d) {}
    TrainAction(const vector<int>& specs) : movement_speed(specs[0]), carry_capacity(specs[1]), harvest_power(specs[2]), chop_power(specs[3]) {}
    string get_command() override {
        return "TRAIN " + to_string(movement_speed) + " " + to_string(carry_capacity) + " " + to_string(harvest_power) + " " + to_string(chop_power);
    }
};
class MineAction : public Action {
public:
    int troll_id;
    MineAction(int id) : troll_id(id) {}
    string get_command() override {
        return "MINE " + to_string(troll_id);
    }
};
class WaitAction : public Action {
public:
    WaitAction() {}
    string get_command() override {
        return "WAIT";
    }
};


class State {
public:
    Player me, opp;
    vector<Tree> trees;

    void read() {
        me.inv.read();
        opp.inv.read();

        int trees_count; cin >> trees_count; cin.ignore();
        trees.clear();
        trees = vector<Tree> (trees_count);
        for (int i = 0; i < trees_count; i++) trees[i].read();

        int trolls_count;
        cin >> trolls_count; cin.ignore();
        vector<Troll> trolls(trolls_count);
        me.trolls.clear();
        opp.trolls.clear();
        for (int i = 0; i < trolls_count; i++) {
            trolls[i].read();
            if (trolls[i].player == 0) me.trolls.push_back(trolls[i]);
            else opp.trolls.push_back(trolls[i]);
        }
    }
};

class Game {
public:
    int turn = 0;
    int W, H;
    std::vector<std::vector<int>> grid;
    Point my_shack, opp_shack;
    vector<vector<int>> distance;
    unordered_map<int, int> sticky_tree_target;   // troll id -> tree index
    unordered_map<int, Point> sticky_cell_target; // troll id -> cell target

    State state;

    void read() {
        cin >> W >> H; cin.ignore();
        grid = vector<vector<int>> (W, vector<int> (H));
        for (int y = 0; y < H; y++) {
            string line; getline(cin, line);
            // . for GRASS, ~ for WATER, # for ROCK, + for IRON, 0 for your own SHACK, 1 for your opponent's SHACK
            for (int x = 0; x < W; x++) {
                char ch = line[x];
                int item = 0;
                if (ch == '.') {
                    item = GRASS;
                } else if (ch == '~') {
                    item = WATER;
                } else if (ch == '#') {
                    item = ROCK;
                } else if (ch == '+') {
                    item = IRON;
                } else if (ch == '0') {
                    item = MY_SHACK;
                    my_shack = Point(x, y);
                } else if (ch == '1') {
                    item = OPP_SHACK;
                    opp_shack = Point(x, y);
                }
                grid[x][y] = item;
            }
        }
        pre_compute_bfs_distance();
    }

    inline int combine_xy(const Point &p) const { return p.x + p.y*W; }

    inline int get_cell_item(const Point &p) const { return grid[p.x][p.y]; }

    inline bool in_bounds(const Point &p) const { return p.x >= 0 && p.y >= 0 && p.x < W && p.y < H; }

    inline bool is_adjacent(const Point &a, const Point &b) const { return manhattan(a, b) == 1; }

    inline int planned_total_trolls() const { return PLANED_TROLLS; }

    inline bool is_walkable(const Point &p) const { return in_bounds(p) && get_cell_item(p) == GRASS; }

    inline bool training_complete() const { return (int)state.me.trolls.size() >= planned_total_trolls(); }

    inline bool adjacent_to_shack(const Point& p) const { return manhattan(p, my_shack) == 1; }
    
    // Returns {moveSpeed, carryCapacity, harvestPower, chopPower} based on current team size
    // plum, lemon, apple, iron
    vector<int> get_next_troll_specs() const {
        int n = (int)state.me.trolls.size();
        if (n >= planned_total_trolls()) return {0, 0, 0, 0}; // No more trolls needed
        if (n == 1) return {2, 2, 1, 1}; // First trained: Balanced Hybrid
        if (n == 2) return {2, 3, 1, 2}; // Second trained: Heavy Gatherer
        // if (n == 3) return {2, 3, 1, 2}; // Third trained: Lumberjack
        return {2, 2, 1, 1};             // Fallback
    }

    bool adjacent_to_iron(const Point& p) const {
        for (const Point& d : p.neighbors()) {
            if (!in_bounds(d)) continue;
            if (get_cell_item(d) == IRON) return true;
        }
        return false;
    }

    bool has_tree_at(const Point& p) const {
        for (const Tree& t : state.trees) {
            if (t.pos == p) return true;
        }
        return false;
    }

    const Tree* tree_at(const Point& p) const {
        for (const Tree& t : state.trees) {
            if (t.pos == p) return &t;
        }
        return nullptr;
    }

    bool occupied_by_my_troll(const Point& p) const {
        for (const Troll& t : state.me.trolls) {
            if (t.pos == p) return true;
        }
        return false;
    }

    bool near_water(const Point& p) const {
        for (const Point& d : p.neighbors()) {
            if (!in_bounds(d)) continue;
            if (get_cell_item(d) == WATER) return true;
        }
        return false;
    }

    Phase get_phase() const {
        if (turn <= 30) return Phase::Planting;
        if ((int)state.me.trolls.size() < planned_total_trolls() && turn <= 150) return Phase::Training;
        if (turn <= 200) return Phase::Harvest;
        return Phase::Mixed;
    }

    bool planted_type_exists_near_shack(int type) const {
        for (const Tree& tree : state.trees) {
            if (get_distance_bw(tree.pos, my_shack) <= 3 && tree.type == type) return true;
        }
        return false;
    }
    int plant_type_score(int type) const {
        int invCnt[4] = {state.me.inv.plum, state.me.inv.lemon, state.me.inv.apple, state.me.inv.banana};
        int score = 100 - invCnt[type] * 20;
        if (!planted_type_exists_near_shack(type)) score += 80;
        return score;
    }
    int best_carried_fruit_type(const Troll& t) const {
        int cnt[4] = {t.carry_plum, t.carry_lemon, t.carry_apple, t.carry_banana};
        int best = -1;
        int bestScore = -1000000;

        for (int i = 0; i < 4; i++) {
            if (cnt[i] <= 0) continue;
            int score = plant_type_score(i);
            if (score > bestScore) {
                bestScore = score;
                best = i;
            }
        }
        return best;
    }

    int best_seed_type_to_pick() const {
        int cnt[4] = {state.me.inv.plum, state.me.inv.lemon, state.me.inv.apple, state.me.inv.banana};
        int best = -1;
        int bestScore = -1;
        for (int i = 0; i < 4; i++) {
            if (cnt[i] <= 0) continue;
            int score = cnt[i];
            // Prefer a type that is not already represented near shack
            if (!planted_type_exists_near_shack(i)) score += 100;
            if (score > bestScore) {
                bestScore = score;
                best = i;
            }
        }
        return best;
    }

    vector<Point> open_cells_near_shack() const {
        vector<Point> res;
        for (int y = 0; y < H; y++) {
            for (int x = 0; x < W; x++) {
                Point p(x, y);

                if (!is_walkable(p)) continue;
                if (has_tree_at(p)) continue;
                if (occupied_by_my_troll(p)) continue;

                int d = get_distance_bw(p, my_shack);
                if (d >= 1 && d <= 3) {
                    res.push_back(p);
                }
            }
        }

        sort(res.begin(), res.end(), [&](const Point& a, const Point& b) {
            int da = get_distance_bw(a, my_shack);
            int db = get_distance_bw(b, my_shack);
            if (da != db) return da < db;

            bool wa = near_water(a);
            bool wb = near_water(b);
            if (wa != wb) return wa > wb;

            if (a.x != b.x) return a.x < b.x;
            return a.y < b.y;
        });

        return res;
    }

    bool contains_point(const vector<Point>& v, const Point& p) const {
        for (const Point& q : v) if (q == p) return true;
        return false;
    }

    int choose_gardener_id() const {
        if (state.me.trolls.empty()) return -1;
        int bestId = -1;
        int bestScore = INT_MAX;
        for (const Troll& t : state.me.trolls) {
            int score = distance_to_shack(t.pos) * 10 + t.total_carry();
            if (t.carry_any_fruit()) {
                score -= 100;
            }
            if (score < bestScore) {
                bestScore = score;
                bestId = t.id;
            }
        }
        return bestId;
    }

    // searches whole map but strongly prefers cells near the shack/water
    Point choose_plant_cell(const Troll& t, const vector<Point>& usedTargets) const {
        Point best(-1, -1);
        int bestScore = INT_MIN;

        for (int y = 0; y < H; y++) {
            for (int x = 0; x < W; x++) {
                Point p(x, y);

                if (!is_cell_plantable_now(p)) continue;
                if (!(p == t.pos) && occupied_by_my_troll(p)) continue;
                if (has_tree_at(p)) continue;
                if (contains_point(usedTargets, p)) continue;

                int d = distance_to_shack(p);

                // Keep planting close to shack, but allow a little flexibility.
                if (d > 6) continue;

                int score = score_plant_cell(p);

                // Slight bonus if the troll can get there quickly.
                score -= get_distance_bw(t.pos, p) * 5;

                if (score > bestScore) {
                    bestScore = score;
                    best = p;
                }
            }
        }
        return best;
    }

    Point choose_best_iron_spot() const {
        Point best(-1, -1);
        int bestDist = INT_MAX;

        for (int y = 0; y < H; y++) {
            for (int x = 0; x < W; x++) {
                Point p(x, y);
                if (!is_walkable(p)) continue;
                if (has_tree_at(p)) continue;
                // if (occupied_by_my_troll(p)) continue;
                if (!adjacent_to_iron(p)) continue;

                int d = get_distance_bw(p, my_shack);
                if (d < bestDist) {
                    bestDist = d;
                    best = p;
                }
            }
        }
        return best;
    }

    int choose_miner_id(const Point& ironSpot) const {
        if (ironSpot.x == -1) return -1;

        int bestId = -1;
        int bestScore = INT_MAX;

        for (const Troll& t : state.me.trolls) {
            if (t.chop_power <= 0 && t.free_capacity() <= 0) continue; // must be able to mine
            int score = get_distance_bw(t.pos, ironSpot) * 100 / t.movement_speed;
            if (score < bestScore) {
                bestScore = score;
                bestId = t.id;
            }
        }
        return bestId;
    }

    int score_plant_cell(const Point& p) const {
        int score = 0;
        score += 100 - distance_to_shack(p) * 20;
        score += near_water(p) ? 100 : 0;
        score += has_tree_at(p) ? -100000 : 0;
        // score += occupied_by_my_troll(p) ? -1000 : 0;
        return score;
    }

    int score_harvest_tree(const Troll& t, const Tree& tr) const {
        int d = get_distance_bw(t.pos, tr.pos);
        int takeNow = min(t.free_capacity(), tr.fruits);

        int score = 0;
        score += takeNow * 160;                 // immediate gain matters most
        score += tr.fruits * 40;                // more fruits is better
        score += tr.size * 20;                  // bigger tree usually better
        score += near_water(tr.pos) ? 35 : 0;   // water trees are premium
        score += (tr.size == 4 ? 40 : (tr.size - 1) * 10);  // mature trees are strong
        score -= d * 20;                        // travel is expensive
        score -= tr.cooldown * 4;               // waiting on cooldown is bad

        if (takeNow == 0) score -= 1000;
        return score;
    }

    int score_chop_tree(const Troll& t, const Tree& tr) const {
        int d = get_distance_bw(t.pos, tr.pos);
        int opp_dist = get_distance_bw(opp_shack, tr.pos);

        int score = 0;
        score += tr.size * 200;                 // wood value is big
        score -= tr.health * 10;                // harder trees are less attractive
        score -= d * 60;                        // endgame cares a lot about distance
        score += (tr.fruits == 0 ? 80 : -20);   // only chop fruitless trees unless desperate

        // the closer the tree is to the opponent's shack, the more we want to chop it to deny them
        score += max(0, 10 - opp_dist) * 1000;

        return score;
    }

    int choose_best_harvest_tree(const Troll& t, const vector<bool>& used) const {
        int best = -1;
        int bestScore = INT_MIN;

        for (int i = 0; i < (int)state.trees.size(); i++) {
            if (used[i]) continue;
            const Tree& tr = state.trees[i];

            // if (tr.fruits <= 0) continue;
            int d = get_distance_bw(t.pos, tr.pos);
            if (tr.fruits == 0 && tr.cooldown > d) continue; // Only ignore if it won't be ready when we arrive

            int sc = score_harvest_tree(t, tr);
            if (sc > bestScore) {
                bestScore = sc;
                best = i;
            }
        }
        return best;
    }

    int choose_best_chop_tree(const Troll& t, const vector<bool>& used) const {
        int best = -1;
        int bestScore = INT_MIN;

        for (int i = 0; i < (int)state.trees.size(); i++) {
            if (used[i]) continue;
            const Tree& tr = state.trees[i];

            int sc = score_chop_tree(t, tr);
            if (sc > bestScore) {
                bestScore = sc;
                best = i;
            }
        }
        return best;
    }

    vector<Point> shack_access_cells() const {
        vector<Point> res;
        for (const Point& d : my_shack.neighbors()) {
            if (in_bounds(d) && is_walkable(d)) res.push_back(d);
        }
        return res;
    }

    inline bool is_shack_access_cell(const Point& p) const { return manhattan(p, my_shack) == 1 && is_walkable(p); }

    int distance_to_shack(const Point& p) const {
        if (p == my_shack) return 0;
        int best = INT_MAX;
        for (const Point& c : shack_access_cells()) {
            best = min(best, get_distance_bw(p, c));
        }
        return best;
    }

    Point choose_best_shack_access_cell(const Point& from) const {
        Point best(-1, -1);
        int bestDist = INT_MAX;

        for (const Point& c : shack_access_cells()) {
            if (occupied_by_my_troll(c)) continue;
            int d = get_distance_bw(from, c);
            if (d < bestDist) {
                bestDist = d;
                best = c;
            }
        }
        return best;
    }

    inline bool is_cell_plantable_now(const Point& p) const { return is_walkable(p) && !has_tree_at(p); }

    inline bool is_valid_tree_target(int idx) const { return idx >= 0 && idx < (int)state.trees.size() && state.trees[idx].fruits > 0; }


    struct TrainingNeed {
        int plum = 0;
        int lemon = 0;
        int apple = 0;
        int iron = 0;
    };
    TrainingNeed get_remaining_training_need() const {
        TrainingNeed need;
        int n = (int)state.me.trolls.size();
        int target = planned_total_trolls();
        if (n >= target) return need; // no more needed

        vector<int> specs = get_next_troll_specs();
        need.plum  = n + specs[0] * specs[0]; // movementSpeed
        need.lemon = n + specs[1] * specs[1]; // carryCapacity
        need.apple = n + specs[2] * specs[2]; // harvestPower
        need.iron  = n + specs[3] * specs[3]; // chopPower

        return need;
    }
    bool can_train_now() const {
        TrainingNeed need = get_remaining_training_need();
        return state.me.inv.plum  >= need.plum &&
            state.me.inv.lemon >= need.lemon &&
            state.me.inv.apple >= need.apple &&
            state.me.inv.iron  >= need.iron;
    }

    int resource_deficit_plum() const {
        TrainingNeed need = get_remaining_training_need();
        return max(0, need.plum - state.me.inv.plum);
    }
    int resource_deficit_lemon() const {
        TrainingNeed need = get_remaining_training_need();
        return max(0, need.lemon - state.me.inv.lemon);
    }
    int resource_deficit_apple() const {
        TrainingNeed need = get_remaining_training_need();
        return max(0, need.apple - state.me.inv.apple);
    }
    int resource_deficit_iron() const {
        TrainingNeed need = get_remaining_training_need();
        return max(0, need.iron - state.me.inv.iron);
    }

    int training_resource_priority(int type) const {
        if (type == PLUM_TYPE)  return resource_deficit_plum();
        if (type == LEMON_TYPE) return resource_deficit_lemon();
        if (type == APPLE_TYPE) return resource_deficit_apple();
        return 0;
    }

    int score_training_tree(const Troll& t, const Tree& tr) const {
        if (tr.fruits <= 0) return -1000000;

        int needed = training_resource_priority(tr.type);
        if (needed <= 0) return -5000; // not useful for training

        int d = get_distance_bw(t.pos, tr.pos);

        int score = 0;
        score += needed * 200;      // shortage matters a lot
        score += tr.fruits * 80;    // more fruits is better
        score += near_water(tr.pos) ? 40 : 0;
        score += tr.size * 15;
        score -= d * 18;
        score -= tr.cooldown * 5;
        return score;
    }
    int choose_best_training_tree(const Troll& t, const vector<bool>& used) const {
        int best = -1;
        int bestScore = INT_MIN;
        for (int i = 0; i < (int)state.trees.size(); i++) {
            if (used[i]) continue;
            const Tree& tr = state.trees[i];
            int sc = score_training_tree(t, tr);
            if (sc > bestScore) {
                bestScore = sc;
                best = i;
            }
        }
        return best;
    }

    inline bool training_needs_iron() const { return resource_deficit_iron() > 0; }


    void pre_compute_bfs_distance() {
        distance.clear();
        distance = vector<vector<int>> (W*H, vector<int> (W*H, 1000000));
        for (int i = 0; i < grid.size(); i++) {
            for (int j = 0; j < grid[0].size(); j++) {
                Point p = Point(i, j);
                vector<vector<bool>> vis(W, vector<bool> (H));
                queue<Point> q;
                auto &dist = distance[combine_xy(p)];
                dist[combine_xy(p)] = 0;
                vis[p.x][p.y] = 1;
                q.push(p);
                while (!q.empty()) {
                    auto u = q.front(); q.pop();
                    for (auto v : u.neighbors()) {
                        if (in_bounds(v) && !vis[v.x][v.y]) {
                            vis[v.x][v.y] = 1;
                            dist[combine_xy(v)] = 1 + dist[combine_xy(u)];
                            if (is_walkable(v)) q.push(v);
                        }
                    }
                }
            }
        }
    }

    int get_distance_bw(const Point &a, const Point &b) const {
        if (a == b) return 0;
        if (a == my_shack) {
            int best = INT_MAX;
            for (const Point& c : shack_access_cells()) best = min(best, 1 + get_distance_bw(c, b));
            return best;
        }
        if (b == my_shack) {
            int best = INT_MAX;
            for (const Point& c : shack_access_cells()) best = min(best, 1 + get_distance_bw(a, c));
            return best;
        }
        return distance[combine_xy(a)][combine_xy(b)];
    }

    Point get_next_step_towards(Point current, Point target) const {
        if (current == target || is_adjacent(current, target)) return target;
        
        Point best_step = current;
        int min_dist = get_distance_bw(current, target);
        
        for (Point n : current.neighbors()) {
            if (is_walkable(n)) {
                int dist = get_distance_bw(n, target);
                if (dist < min_dist) {
                    min_dist = dist;
                    best_step = n;
                }
            }
        }
        return best_step;
    }


    bool should_return_home_to_drop(const Troll& troll) const {
        // Wood is score, so return home immediately if carrying any wood.
        if (troll.carry_wood > 0) return true;

        // Also return home if carrying too much of anything.
        // return troll.total_carry() >= max(1, troll.carry_capacity * 7 / 10);
        return troll.is_full();
    }




    vector<unique_ptr<Action>> find_best_moves_for_all() {
        vector<unique_ptr<Action>> actions;
        vector<bool> usedTree(state.trees.size(), false);
        vector<Point> usedPlantTargets;

        Phase phase = get_phase();


        auto training_need_of = [&](int type) -> int {
            int n = (int)state.me.trolls.size();
            int target = planned_total_trolls(); // start + 2 extra trolls
            if (target <= n) return 0;

            vector<int> specs = get_next_troll_specs();
            
            int required = 0;
            if (type == PLUM_TYPE) required += n + specs[0] * specs[0];
            else if (type == LEMON_TYPE) required += n + specs[1] * specs[1];
            else if (type == APPLE_TYPE) required += n + specs[2] * specs[2];

            int have = 0;
            if (type == PLUM_TYPE) have = state.me.inv.plum;
            else if (type == LEMON_TYPE) have = state.me.inv.lemon;
            else if (type == APPLE_TYPE) have = state.me.inv.apple;

            return max(0, required - have);
        };

        auto score_training_tree = [&](const Troll& troll, const Tree& tr) -> int {
            if (tr.fruits <= 0) return -1000000;

            int need = training_need_of(tr.type);
            if (need <= 0) return -5000; // not useful for current training plan

            int d = get_distance_bw(troll.pos, tr.pos);

            int score = 0;
            score += need * 250;                 // shortage matters the most
            score += tr.fruits * 80;             // more fruits is better
            score += tr.size * 20;
            score += near_water(tr.pos) ? 40 : 0;
            score -= d * 18;
            score -= tr.cooldown * 5;

            return score;
        };

        auto choose_best_training_tree = [&](const Troll& troll) -> int {
            int best = -1;
            int bestScore = INT_MIN;

            for (int i = 0; i < (int)state.trees.size(); i++) {
                const Tree& tr = state.trees[i];

                if (tr.type > APPLE_TYPE) continue; // ignore BANANA for the current training plan

                int sc = score_training_tree(troll, tr);
                if (sc > bestScore) {
                    bestScore = sc;
                    best = i;
                }
            }

            return best;
        };

        auto get_sticky_tree = [&](int id) -> int {
            auto it = sticky_tree_target.find(id);
            if (it == sticky_tree_target.end()) return -1;
            int idx = it->second;
            if (!is_valid_tree_target(idx) || usedTree[idx]) return -1;
            return idx;
        };

        auto set_sticky_tree = [&](int id, int idx) { sticky_tree_target[id] = idx; };

        auto get_sticky_cell = [&](int id) -> Point {
            auto it = sticky_cell_target.find(id);
            if (it == sticky_cell_target.end()) return Point(-1, -1);
            return it->second;
        };

        auto set_sticky_cell = [&](int id, const Point& p) { sticky_cell_target[id] = p; };

        int gardenerId = (phase == Phase::Planting) ? choose_gardener_id() : -1;

        // Determine if we need an iron miner this turn
        Point ironSpot = Point(-1, -1);
        int minerId = -1;
        if (phase == Phase::Training && training_needs_iron()) {
            ironSpot = choose_best_iron_spot();
            if (ironSpot.x != -1) {
                minerId = choose_miner_id(ironSpot);
            }
        }

        for (const Troll& troll : state.me.trolls) {
            if (!sticky_tree_target.count(troll.id)) sticky_tree_target[troll.id] = -1;
            if (!sticky_cell_target.count(troll.id)) sticky_cell_target[troll.id] = Point(-1, -1);

            bool isGardener = (troll.id == gardenerId);

            // -------------------------------------------------
            // PHASE 1: PLANTING
            // -------------------------------------------------
            if (phase == Phase::Planting && isGardener) {
                int carriedFruitType = best_carried_fruit_type(troll);

                if (carriedFruitType != -1) {
                    Point plantCell = choose_plant_cell(troll, usedPlantTargets);

                    if (plantCell.x != -1) {
                        usedPlantTargets.push_back(plantCell);
                        sticky_tree_target[troll.id] = -1;
                        set_sticky_cell(troll.id, plantCell);

                        if (troll.pos == plantCell) {
                            actions.push_back(make_unique<PlantAction>(troll.id, carriedFruitType));
                        } else {
                            // actions.push_back(make_unique<MoveAction>(troll.id, get_next_step_towards(troll.pos, plantCell)));
                            actions.push_back(make_unique<MoveAction>(troll.id, plantCell));
                        }
                    } else {
                        // No good planting spot left, fallback to fruit harvesting
                        int idx = get_sticky_tree(troll.id);
                        if (idx == -1) idx = choose_best_harvest_tree(troll, usedTree);

                        if (idx != -1) {
                            set_sticky_tree(troll.id, idx);
                            usedTree[idx] = true;
                            set_sticky_cell(troll.id, state.trees[idx].pos);

                            if (troll.pos == state.trees[idx].pos) {
                                actions.push_back(make_unique<HarvestAction>(troll.id));
                            } else {
                                // actions.push_back(make_unique<MoveAction>(troll.id, get_next_step_towards(troll.pos, state.trees[idx].pos)));
                                actions.push_back(make_unique<MoveAction>(troll.id, state.trees[idx].pos));
                            }
                        } else {
                            actions.push_back(make_unique<WaitAction>());
                        }
                    }

                    continue;
                }

                int pickType = best_seed_type_to_pick();
                if (pickType != -1) {
                    if (is_shack_access_cell(troll.pos)) {
                        sticky_tree_target[troll.id] = -1;
                        actions.push_back(make_unique<PickAction>(troll.id, pickType));
                    } else {
                        Point homeCell = get_sticky_cell(troll.id);
                        if (homeCell.x == -1 || !is_shack_access_cell(homeCell)) {
                            homeCell = choose_best_shack_access_cell(troll.pos);
                            set_sticky_cell(troll.id, homeCell);
                        }

                        if (homeCell.x != -1) {
                            sticky_tree_target[troll.id] = -1;
                            // actions.push_back(make_unique<MoveAction>(troll.id, get_next_step_towards(troll.pos, homeCell)));
                            actions.push_back(make_unique<MoveAction>(troll.id, homeCell));
                        } else {
                            actions.push_back(make_unique<WaitAction>());
                        }
                    }

                    continue;
                }

                // No seeds, no carried fruit: use this troll to harvest something useful
                int idx = get_sticky_tree(troll.id);
                if (idx == -1) idx = choose_best_harvest_tree(troll, usedTree);

                if (idx != -1) {
                    set_sticky_tree(troll.id, idx);
                    usedTree[idx] = true;
                    set_sticky_cell(troll.id, state.trees[idx].pos);

                    if (troll.pos == state.trees[idx].pos) {
                        actions.push_back(make_unique<HarvestAction>(troll.id));
                    } else {
                        // actions.push_back(make_unique<MoveAction>(troll.id, get_next_step_towards(troll.pos, state.trees[idx].pos)));
                        actions.push_back(make_unique<MoveAction>(troll.id, state.trees[idx].pos));
                    }
                } else {
                    actions.push_back(make_unique<WaitAction>());
                }

                continue;
            }

            // -------------------------------------------------
            // PHASE 2: TRAINING
            // -------------------------------------------------
            if (phase == Phase::Training) {
                // If carrying too much, go drop it first
                if (should_return_home_to_drop(troll)) {
                    if (is_shack_access_cell(troll.pos)) {
                        sticky_tree_target[troll.id] = -1;
                        actions.push_back(make_unique<DropAction>(troll.id));
                    } else {
                        Point homeCell = get_sticky_cell(troll.id);
                        if (homeCell.x == -1 || !is_shack_access_cell(homeCell)) {
                            homeCell = choose_best_shack_access_cell(troll.pos);
                            set_sticky_cell(troll.id, homeCell);
                        }

                        sticky_tree_target[troll.id] = -1;

                        if (homeCell.x != -1) {
                            // actions.push_back(make_unique<MoveAction>(troll.id, get_next_step_towards(troll.pos, homeCell)));
                            actions.push_back(make_unique<MoveAction>(troll.id, homeCell));
                        } else {
                            actions.push_back(make_unique<WaitAction>());
                        }
                    }
                    continue;
                }

                // Execute Mining if this troll is the chosen miner
                if (troll.id == minerId && training_needs_iron()) {
                    cerr << "Miner " << troll.id << " targeting iron at " << ironSpot.x << "," << ironSpot.y << endl;
                    if (!troll.is_full()) {
                        ironSpot = choose_best_iron_spot();
                        if (adjacent_to_iron(troll.pos)) {
                            actions.push_back(make_unique<MineAction>(troll.id));
                            sticky_tree_target[troll.id] = -1;
                        } else if (ironSpot.x != -1) {
                            set_sticky_cell(troll.id, ironSpot);
                            sticky_tree_target[troll.id] = -1;
                            // actions.push_back(make_unique<MoveAction>(troll.id, get_next_step_towards(troll.pos, ironSpot)));
                            actions.push_back(make_unique<MoveAction>(troll.id, ironSpot));
                        } else {
                            actions.push_back(make_unique<WaitAction>());
                        }
                    }
                    continue;
                }
                
                // If training resources are already enough, do not waste time on random harvesting.
                // Just keep workers productive, but only on useful trees.
                int idx = get_sticky_tree(troll.id);
                if (idx == -1) idx = choose_best_training_tree(troll);

                if (idx != -1 && training_need_of(state.trees[idx].type) > 0) {
                    set_sticky_tree(troll.id, idx);
                    usedTree[idx] = true;
                    set_sticky_cell(troll.id, state.trees[idx].pos);

                    if (troll.pos == state.trees[idx].pos) {
                        actions.push_back(make_unique<HarvestAction>(troll.id));
                    } else {
                        // actions.push_back(make_unique<MoveAction>(troll.id, get_next_step_towards(troll.pos, state.trees[idx].pos)));
                        actions.push_back(make_unique<MoveAction>(troll.id, state.trees[idx].pos));
                    }
                } else {
                    // Fallback: useful harvest
                    int h = choose_best_harvest_tree(troll, usedTree);
                    if (h != -1) {
                        set_sticky_tree(troll.id, h);
                        usedTree[h] = true;
                        set_sticky_cell(troll.id, state.trees[h].pos);

                        if (troll.pos == state.trees[h].pos) {
                            actions.push_back(make_unique<HarvestAction>(troll.id));
                        } else {
                            // actions.push_back(make_unique<MoveAction>(troll.id, get_next_step_towards(troll.pos, state.trees[h].pos)));
                            actions.push_back(make_unique<MoveAction>(troll.id, state.trees[h].pos));
                        }
                    } else {
                        actions.push_back(make_unique<WaitAction>());
                    }
                }

                continue;
            }

            // -------------------------------------------------
            // PHASE 3: HARVEST
            // -------------------------------------------------
            if (phase == Phase::Harvest || (phase == Phase::Mixed && troll.chop_power == 0)) {
                if (should_return_home_to_drop(troll)) {
                    if (is_shack_access_cell(troll.pos)) {
                        sticky_tree_target[troll.id] = -1;
                        actions.push_back(make_unique<DropAction>(troll.id));
                    } else {
                        Point homeCell = get_sticky_cell(troll.id);
                        if (homeCell.x == -1 || !is_shack_access_cell(homeCell)) {
                            homeCell = choose_best_shack_access_cell(troll.pos);
                            set_sticky_cell(troll.id, homeCell);
                        }

                        sticky_tree_target[troll.id] = -1;

                        if (homeCell.x != -1) {
                            // actions.push_back(make_unique<MoveAction>(troll.id, get_next_step_towards(troll.pos, homeCell)));
                            actions.push_back(make_unique<MoveAction>(troll.id, homeCell));
                        } else {
                            actions.push_back(make_unique<WaitAction>());
                        }
                    }

                    continue;
                }

                int idx = get_sticky_tree(troll.id);
                if (idx == -1) idx = choose_best_harvest_tree(troll, usedTree);

                if (idx != -1) {
                    set_sticky_tree(troll.id, idx);
                    usedTree[idx] = true;
                    set_sticky_cell(troll.id, state.trees[idx].pos);

                    if (troll.pos == state.trees[idx].pos) {
                        actions.push_back(make_unique<HarvestAction>(troll.id));
                    } else {
                        // actions.push_back(make_unique<MoveAction>(troll.id, get_next_step_towards(troll.pos, state.trees[idx].pos)));
                        actions.push_back(make_unique<MoveAction>(troll.id, state.trees[idx].pos));
                    }
                } else {
                    actions.push_back(make_unique<WaitAction>());
                }

                continue;
            }

            // -------------------------------------------------
            // PHASE 4: MIXED ENDGAME
            // Only trolls with chop_power > 0 may chop.
            // -------------------------------------------------
            if (phase == Phase::Mixed) {
                // 1) If the troll is carrying wood or is too full, return home first.
                if (should_return_home_to_drop(troll)) {
                    if (is_shack_access_cell(troll.pos)) {
                        sticky_tree_target[troll.id] = -1;
                        actions.push_back(make_unique<DropAction>(troll.id));
                    } else {
                        Point homeCell = get_sticky_cell(troll.id);
                        if (homeCell.x == -1 || !is_shack_access_cell(homeCell)) {
                            homeCell = choose_best_shack_access_cell(troll.pos);
                            set_sticky_cell(troll.id, homeCell);
                        }

                        sticky_tree_target[troll.id] = -1;

                        if (homeCell.x != -1) {
                            // actions.push_back(make_unique<MoveAction>(troll.id, get_next_step_towards(troll.pos, homeCell)));
                            actions.push_back(make_unique<MoveAction>(troll.id, homeCell));
                        } else {
                            actions.push_back(make_unique<WaitAction>());
                        }
                    }

                    continue;
                }

                // 2) No need to drop now. Decide chop vs harvest.
                if (troll.chop_power > 0) {
                    int chopIdx = -1;
                    int chopBestScore = INT_MIN;

                    for (int i = 0; i < (int)state.trees.size(); i++) {
                        if (usedTree[i]) continue;
                        const Tree& tr = state.trees[i];

                        int sc = score_chop_tree(troll, tr);
                        if (sc > chopBestScore) {
                            chopBestScore = sc;
                            chopIdx = i;
                        }
                    }

                    int harvestIdx = choose_best_harvest_tree(troll, usedTree);
                    int harvestScore = INT_MIN;
                    if (harvestIdx != -1) {
                        harvestScore = score_harvest_tree(troll, state.trees[harvestIdx]);
                    }

                    // Prefer chopping only if it is clearly better than harvesting.
                    // Also prefer chopping fruitless trees.
                    bool goodChop =
                        chopIdx != -1 &&
                        (state.trees[chopIdx].fruits == 0 || chopBestScore >= harvestScore + 30);

                    if (goodChop) {
                        set_sticky_tree(troll.id, chopIdx);
                        usedTree[chopIdx] = true;
                        set_sticky_cell(troll.id, state.trees[chopIdx].pos);

                        if (troll.pos == state.trees[chopIdx].pos) {
                            actions.push_back(make_unique<ChopAction>(troll.id));
                        } else {
                            // actions.push_back(make_unique<MoveAction>(troll.id, get_next_step_towards(troll.pos, state.trees[chopIdx].pos)));
                            actions.push_back(make_unique<MoveAction>(troll.id, state.trees[chopIdx].pos));
                        }
                    } else if (harvestIdx != -1) {
                        set_sticky_tree(troll.id, harvestIdx);
                        usedTree[harvestIdx] = true;
                        set_sticky_cell(troll.id, state.trees[harvestIdx].pos);

                        if (troll.pos == state.trees[harvestIdx].pos) {
                            actions.push_back(make_unique<HarvestAction>(troll.id));
                        } else {
                            // actions.push_back(make_unique<MoveAction>(troll.id, get_next_step_towards(troll.pos, state.trees[harvestIdx].pos)));
                            actions.push_back(make_unique<MoveAction>(troll.id, state.trees[harvestIdx].pos));
                        }
                    } else {
                        actions.push_back(make_unique<WaitAction>());
                    }
                }
                continue;
            }
        }

        // Train immediately when the next troll is affordable.
        if (phase == Phase::Training && (int)state.me.trolls.size() < planned_total_trolls() && can_train_now()) {
            actions.push_back(make_unique<TrainAction>(get_next_troll_specs()));
        }

        return actions;
    }


    void output() {
        vector<unique_ptr<Action>> actions = find_best_moves_for_all();
        int n = actions.size();
        for (int i = 0; i < n; i++) {
            cout << actions[i] -> get_command();
            if (i < n - 1) {
                cout << ";";
            }
        }
        cout << endl;
    }
};

int main() {
    Game game;
    game.read();

    while (1) {
        game.turn++;
        game.state.read();
        game.output();
    }
}
