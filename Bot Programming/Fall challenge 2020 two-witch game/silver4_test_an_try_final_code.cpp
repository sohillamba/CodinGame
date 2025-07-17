/*
Not much changes, just tried to pack 4 ingridients in one 32 bit numbers, but was buggy, still buggy. 
Will fix and improve later.
Right now stopping on this challenge
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
#include <unordered_set>
#include <functional>

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

// struct Pos {
//     // just storing 4 numbers (after adding 32) at 4 bytes(8bits)
//     // ---d+32---|---c+32---|---b+32---|---a+32---
//     // -----8----|-----8----|-----8----|-----8----
//     uint32_t p;
//     constexpr Pos(): p(0) {}
//     constexpr Pos(uint32_t x) : p(x) {}
//     constexpr Pos(int a, int b, int c, int d) {
//         p = ((uint32_t(a + 32))) + 
//             ((uint32_t(b + 32)) << 8) + 
//             ((uint32_t(c + 32)) << 16) + 
//             ((uint32_t(d + 32)) << 24); 
//     }

//     inline void add(const Pos &o) { p = p + o.p - 0x20202020; }
//     inline void minus(const Pos &o) { p = p - o.p + 0x20202020; }

//     inline int operator[](int i) const {
//         uint32_t x = (p >> (i*8)) & 0xFF;
//         return x - 32;
//     }

//     inline int total() const {
//         int sum = 0;
//         for (int i = 0; i < 4; ++i) {
//             int v = (*this)[i];
//             sum += v;
//             assert(v >= 0);
//         }

//         int sum2 = fast_total();
//         assert(sum == sum2);

//         return sum;
//     }

//     inline int fast_total() const {
//         return ((p - 0x20202020) * 0x01010101) >> 24;
//     }

//     inline bool is_positive() const {
//         // return ((p & 0x7F7F7F7F) + 0x5F5F5F5F) & 0x80808080 == 0x80808080;
//         // return ((p - 0x20202020) & 0x80808080) == 0;
//         return ((p - 0x20202020) & 0x80808080) == 0x80808080;
//     }

//     inline bool valid() const {
//         return is_positive() & (fast_total() <= 10);
//     }

//     inline operator bool() const {
//         return valid();
//     }

//     inline bool operator>=(const Pos &o) const {
//         return ((p - o.p) & 0x80808080) == 0;
//     }

//     // inline bool operator>(Pos rhs) const {
//     //     return ((o.p - p) & 0x80808080) && !((p - o.p) & 0x80808080);
//     // }

//     inline void multiply (uint32_t t) {
//         // uint32_t adjusted = p - 0x20202020;
//         // uint32_t scaled = ((adjusted * t) / 256) * 0x01010101;
//         // return scaled + 0x20202020;
//         p = ((((p - 0x20202020) * t) / 256) * 0x01010101) + 0x20202020;
//     }

//     // inline void set(int i, uint32_t val) {
//     //     if (i == 0) set0(val);
//     //     else if (i == 1) set1(val);
//     //     else if (i == 2) set2(val);
//     //     else set3(val);
//     // }

//     inline void set0(uint32_t val) {
//         p = (p & 0xFFFFFF00) | (val + 32);
//     }

//     inline void set1(uint32_t val) {
//         p = (p & 0xFFFF00FF) | ((val + 32) << 8);
//     }

//     inline void set2(uint32_t val) {
//         p = (p & 0xFF00FFFF) | ((val + 32) << 16);
//     }

//     inline void set3(uint32_t val) {
//         p = (p & 0x00FFFFFF) | ((val + 32) << 24);
//     }
// };


// struct Pos {
//     int p[4];
//     constexpr Pos() { p[0] = p[1] = p[2] = p[3] = 0; }
//     constexpr Pos(int a, int b, int c, int d) {
//         p[0] = a;
//         p[1] = b;
//         p[2] = c;
//         p[3] = d;
//     }

//     inline void add(const Pos &o) {
//         for (int i = 0; i < 4; i++) {
//             p[i] += o.p[i];
//         }
//     }
//     inline void minus(const Pos &o) {
//         for (int i = 0; i < 4; i++) {
//             p[i] -= o.p[i];
//         }
//     }

//     inline int operator[](int i) const {
//         return p[i];
//     }

//     inline int total() const {
//         int sum = 0;
//         for (int i = 0; i < 4; ++i) {
//             int v = (*this)[i];
//             sum += v;
//         }

//         int sum2 = fast_total();
//         assert(sum == sum2);

//         return sum;
//     }

//     inline int fast_total() const {
//         int sum = 0;
//         for (int i = 0; i < 4; i++) {
//             sum += p[i];
//         }
//         return sum;
//     }

//     inline bool is_positive() const {
//         for (int i = 0; i < 4; i++) {
//             if (p[i] < 0) return false;
//         }
//         return true;
//     }

//     inline bool valid() const {
//         return is_positive() & (fast_total() <= 10);
//     }

//     inline operator bool() const {
//         return valid();
//     }

//     inline bool operator>=(const Pos &o) const {
//         for (int i = 0; i < 4; i++) {
//             if (p[i] < o.p[i]) return false;
//         }
//         return true;
//     }

//     // inline bool operator>(Pos rhs) const {
//     //     return ((o.p - p) & 0x80808080) && !((p - o.p) & 0x80808080);
//     // }

//     inline void multiply (uint32_t t) {
//         for (int i = 0; i < 4; i++) {
//             p[i] *= t;
//         }
//     }

//     // inline void set(int i, uint32_t val) {
//     //     if (i == 0) set0(val);
//     //     else if (i == 1) set1(val);
//     //     else if (i == 2) set2(val);
//     //     else set3(val);
//     // }

//     inline void set0(uint32_t val) {
//         p[0] = val;
//     }

//     inline void set1(uint32_t val) {
//         p[1] = val;
//     }

//     inline void set2(uint32_t val) {
//         p[2] = val;
//     }

//     inline void set3(uint32_t val) {
//         p[3] = val;
//     }
// };


// struct Pos {
//     int p[4];
//     uint32_t p2;
//     constexpr Pos() { p[0] = p[1] = p[2] = p[3] = 0; p2 = 0; }
//     constexpr Pos(int a, int b, int c, int d) {
//         p[0] = a;
//         p[1] = b;
//         p[2] = c;
//         p[3] = d;
        
//         p2 = ((uint32_t(a + 32))) + 
//             ((uint32_t(b + 32)) << 8) + 
//             ((uint32_t(c + 32)) << 16) + 
//             ((uint32_t(d + 32)) << 24);

//         check("constructor");
//     }

//     inline void add(const Pos &o) {
//         for (int i = 0; i < 4; i++) {
//             p[i] += o.p[i];
//         }
//         p2 = p2 + o.p2 - 0x20202020;
//         check("add");
//     }
//     inline void minus(const Pos &o) {
//         for (int i = 0; i < 4; i++) {
//             p[i] -= o.p[i];
//         }
//         p2 = p2 - o.p2 + 0x20202020;
//         check("minus");
//     }

//     inline int operator[](int i) const {
//         uint32_t x = (p2 >> (i*8)) & 0xFF;
//         int ans = x - 32;
//         // assert(ans == p[i]);
//         return ans;
//     }

//     inline int total() const {
//         int sum = 0;
//         for (int i = 0; i < 4; ++i) {
//             int v = (*this)[i];
//             sum += v;
//         }

//         int sum2 = fast_total();
//         int sum3_bits = fast_total2();
//         assert(sum == sum2);
//         assert(sum == sum3_bits);

//         return sum;
//     }

//     inline int fast_total() const {
//         int sum = 0;
//         for (int i = 0; i < 4; i++) {
//             sum += p[i];
//         }
//         return sum;
//     }

//     inline int fast_total2() const {
//         return ((p2 - 0x20202020) * 0x01010101) >> 24;
//     }

//     inline bool is_positive() const {
//         bool res = true;
//         for (int i = 0; i < 4; i++) {
//             if (p[i] < 0) res = false;
//         }

//         bool res2 = ((p2 - 0x20202020) & 0x80808080) == 0x80808080;
//         assert (res == res2);
//         return res;
//     }

//     inline bool valid() const {
//         return is_positive() & (fast_total() <= 10);
//     }

//     inline operator bool() const {
//         return valid();
//     }

//     inline bool operator>=(const Pos &o) const {
//         bool res = true;
//         for (int i = 0; i < 4; i++) {
//             if (p[i] < o.p[i]) res = false;
//         }

//         bool res2 = ((p2 - o.p2) & 0x80808080) == 0;
//         assert(res == res2);

//         return res;
//     }

//     // inline bool operator>(Pos rhs) const {
//     //     return ((o.p - p) & 0x80808080) && !((p - o.p) & 0x80808080);
//     // }

//     inline void multiply (uint32_t t) {
//         for (int i = 0; i < 4; i++) {
//             p[i] *= t;
//         }
//         p2 = ((((p2 - 0x20202020) * t) / 256) * 0x01010101) + 0x20202020;

//         check("multiply");
//     }

//     // inline void set(int i, uint32_t val) {
//     //     if (i == 0) set0(val);
//     //     else if (i == 1) set1(val);
//     //     else if (i == 2) set2(val);
//     //     else set3(val);
//     // }

//     inline void set0(uint32_t val) {
//         p[0] = val;
//         p2 = (p2 & 0xFFFFFF00) | (val + 32);
//         check("set0");
//     }

//     inline void set1(uint32_t val) {
//         p[1] = val;
//         p2 = (p2 & 0xFFFF00FF) | ((val + 32) << 8);
//         check("set1");
//     }

//     inline void set2(uint32_t val) {
//         p[2] = val;
//         p2 = (p2 & 0xFF00FFFF) | ((val + 32) << 16);
//         check("set2");
//     }

//     inline void set3(uint32_t val) {
//         p[3] = val;
//         p2 = (p2 & 0x00FFFFFF) | ((val + 32) << 24);
//         check("set3");
//     }

//     inline void check(string s) {
//         for (int i = 0; i < 4; i++) {
//             if (p[i] != (*this)[i]) {
//                 cerr << s << " : " << endl;
//                 for (int j = 0; j < 4; j++) {
//                     cerr << p[j] << " ";
//                 }
//                 cerr << endl;
//                 for (int j = 0; j < 4; j++) {
//                     cerr << (*this)[j] << " ";
//                 }
//                 cerr << endl;
//                 return;
//             }
//         }
//     }
// };


struct Pos {
    uint32_t p;

    // Per‑lane bias so that raw v in –32..+31 is stored as (v+32) in one byte
    static constexpr uint32_t BIAS       = 32;
    static constexpr uint32_t BIAS_WORD  = 0x20202020u; // 32 in each byte
    static constexpr uint32_t LANE_MASK  = 0x3Fu;       // low 6 bits per byte

    // Default = zero inventory (0 in each lane → stored = 32)
    constexpr Pos() : p(BIAS_WORD) {}

    // Build from an already‑packed lanes (assumed bias=32)
    explicit constexpr Pos(uint32_t x) : p(x) {}

    // Build from four signed ints in –32..+31
    constexpr Pos(int a,int b,int c,int d)
      : p(
          (((uint32_t)(a + BIAS) & 0xFFu) <<  0) |
          (((uint32_t)(b + BIAS) & 0xFFu) <<  8) |
          (((uint32_t)(c + BIAS) & 0xFFu) << 16) |
          (((uint32_t)(d + BIAS) & 0xFFu) << 24)
        )
    {}

    // Lane‑wise add: (v1+v2) stays biased by +32
    inline void add(const Pos &o) const {
        // p = ( (v1+32)+(v2+32) − 32 ) in each lane
        p = p + o.p - BIAS_WORD;
    }
    // Lane‑wise sub: (v1−v2)
    inline void minus(const Pos &o) const {
        // p = ( (v1+32)−(v2+32) + 32 ) in each lane
        p = p - o.p + BIAS_WORD;
    }

    // Extract lane i (0..3), sign‑extend from 6 bits
    inline int operator[](int i) const {
        uint32_t byte = (p >> (i*8)) & 0xFFu;
        int v = int(byte) - int(BIAS);
        return v;
    }

    // Total of all four lanes (assumes you only call on non‑negative inventory)
    inline int total() const {
        // fast dot‑product trick: sum of bytes
        // sum_bytes = ((p * 0x01010101) >> 24)
        // but each byte is (v+32), so sum_bytes = sum(v) + 4*32 = sum(v) + 128
        uint32_t sum_bytes = (p * 0x01010101u) >> 24;
        return int(sum_bytes) - 128;
    }

    inline int fast_total() {
        return total();
    }

    // Test non‑negativity: each lane’s stored byte ≥ 32 → high 7th bit of (byte−32) is 0
    // equivalently: ((p − BIAS_WORD) & 0x80808080) == 0
    inline bool is_non_negative() const {
        return ((p - BIAS_WORD) & 0x80808080u) == 0;
    }

    // Valid inventory: all lanes ≥0 and total ≤10
    inline bool valid() const {
        if (!is_non_negative()) return false;
        int tot = total();
        return tot >= 0 && tot <= MAX_CAPICITY;
    }

    // Boolean test
    inline explicit operator bool() const {
        return valid();
    }

    // Comparison: this ≥ o iff every lane v1 ≥ v2
    inline bool operator>=(Pos o) const {
        // (this−o) must be non‑negative in every lane
        return (((p - o.p) - BIAS_WORD) & 0x80808080u) == 0;
    }

    // // Strict
    // inline bool operator>(Pos o) const {
    //     return p != o.p && *this >= o;
    // }

    // Scale all lanes by t: v_i ← v_i * t
    inline Pos multiply(int t) const {
        // extract, multiply, re‑bias per lane
        uint32_t x = p - BIAS_WORD;
        uint32_t b0 = ((x >>  0) & LANE_MASK) * t;
        uint32_t b1 = ((x >>  8) & LANE_MASK) * t;
        uint32_t b2 = ((x >> 16) & LANE_MASK) * t;
        uint32_t b3 = ((x >> 24) & LANE_MASK) * t;
        // mask back down to 6 bits
        b0 &= LANE_MASK;
        b1 &= LANE_MASK;
        b2 &= LANE_MASK;
        b3 &= LANE_MASK;
        return Pos{
            (b0 <<  0) | (b1 <<  8) |
            (b2 << 16) | (b3 << 24)
        } + Pos(); // add back a single BIAS_WORD
    }

    // Set lane i to v (–32…+31)
    inline void set(int i, int v) {
        uint32_t e = uint32_t((v + BIAS) & 0xFFu);
        uint32_t m = uint32_t(0xFFu) << (i*8);
        p = (p & ~m) | (e << (i*8));
    }
};




class Action {
public:
    int id;             // the unique ID of this spell or recipe
    // string type;        // CAST, OPPONENT_CAST, LEARN, BREW
    // int delta[4];       // ingredient changes
    Pos cost;
    int price;          // the price in rupees if this is a potion
    int tome_index;     // in the first two leagues: always 0; later: the index in the tome if this is a tome spell, equal to the read-ahead tax; For brews, this is the value of the current urgency bonus
    int tax_count;      // in the first two leagues: always 0; later: the amount of taxed tier-0 ingredients you gain from learning this spell; For brews, this is how many times you can still gain an urgency bonus
    bool castable;      // in the first league: always 0; later: 1 if this is a castable player spell
    bool repeatable;    // for the first two leagues: always 0; later: 1 if this is a repeatable player spell
    
    bool completed = false; // already completed this order

    string read() {
        int delta[4];
        string type;
        std::cin >> id >> type >> delta[0] >> delta[1] >> delta[2] >> delta[3];
        std::cin >> price;
        std::cin >> tome_index >> tax_count;
        std::cin >> castable >> repeatable;
        std::cin.ignore();
        cost = Pos(delta[0], delta[1], delta[2], delta[3]);
        return type;
    }
};

class Player {
public:
    int id;
    // int inv[4];
    Pos inv;
    int score;

    int brewed_count = 0;
    int learn_count = 0;

    Player (int id) : id(id) {}

    void read() {
        int _inv[4];
        std::cin >> _inv[0] >> _inv[1] >> _inv[2] >> _inv[3] >> score; std::cin.ignore();
        inv = Pos(_inv[0], _inv[1], _inv[2], _inv[3]);
    }

    bool can_brew(const Action &a) { // condition (a.completed == false) is already tasted via order_mask
        // return inv[0] >= -a.delta[0] && inv[1] >= -a.delta[1] && inv[2] >= -a.delta[2] && inv[3] >= -a.delta[3];
        auto res = a.cost;
        res.add(inv);
        return res.valid();
    }

    bool can_cast(const Action &a, const int t = 1) { // condition (a.castable == true) is already tasted via spell_mask
        // return inv[0] >= -a.delta[0]*t && inv[1] >= -a.delta[1]*t && inv[2] >= -a.delta[2]*t && inv[3] >= -a.delta[3]*t &&
        //     inv[0] + inv[1] + inv[2] + inv[3] + a.delta[0]*t + a.delta[1]*t + a.delta[2]*t + a.delta[3]*t <= MAX_CAPICITY;
        auto res = a.cost;
        res.multiply(t);
        res.add(inv);
        return res.valid();
    }

    uint32_t get_hash() {
        // return inv[0] | (inv[1] << 4) | (inv[2] << 8) | (inv[3] << 12);
        return inv.p;
        // return (inv.p[0]) | (inv.p[1] << 8) + (inv.p[2] << 16) + (inv.p[3] << 24);
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
    
    Pos inv_old;    // player old inv
    int score_old;     // player old score

    int bonus1_old, bonus3_old;     // game old bonuses
    
    uint64_t exhausted_mask_old;    // for REST, for indexes of exahusted spells
};

struct Undo_double_move {
    int type[2];   // move type
    int idx[2];    // action index
    
    Pos inv_old[2];    // player old inv
    int score_old[2];     // player old score

    int bonus1_old, bonus3_old;     // game old bonuses
    
    uint64_t exhausted_mask_old[2];    // for REST, for indexes of exahusted spells
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

    string to_string() {
        return std::to_string((uint64_t)p[0].get_hash() | ((uint64_t)p[1].get_hash() << 32)) + "|" +
            std::to_string(orders_mask) + "|" + 
            std::to_string(spells_mask[0]) + "|" + 
            std::to_string(spells_mask[1]) + "|" +
            std::to_string(tome_spells_mask) + "|" + 
            std::to_string(turn);
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
    // double get_heurestinc_score(const Move &move, const int pid) {
    //     static constexpr int ING_W[4] = {1, 5, 25, 125};
        
    //     // brew > cast > learn > rest > wait
    //     //                  indices order -> brew, cast, rest, wait, learn
    //     static constexpr double TYPE_P[5] = {4000, 3000, 1000, 0, 2000};
        
    //     // tuning factors
    //     static constexpr double PRICE_FACTOR        = 50.0;
    //     static constexpr double COST_FACTOR         = 1.0;
    //     static constexpr double QUEUE_PENALTY       = 1.0;
    //     static constexpr double CAST_GAIN_FACTOR    = 5.0;
    //     static constexpr double CAST_SPENT_PENALTY  = 1.0;
    //     static constexpr double LEARN_GAIN_FACTOR   = 5.0;
    //     static constexpr double LEARN_SPENT_PENALTY = 1.0;
    //     static constexpr double LEARN_COST_PENALTY  = 20.0;

    //     int type = move.get_type();
    //     int idx = move.get_index();
    //     double sc = TYPE_P[type];

    //     if (type == BREW) {
    //         auto &a = orders[idx];
    //         double reward = 0;
    //         for (int t = 0; t < 4; ++t) {
    //             reward += a.delta[t] * ING_W[t];
    //         }
    //         sc += a.price * PRICE_FACTOR
    //             + reward * COST_FACTOR
    //             - move.tax * QUEUE_PENALTY;
    //     }
    //     else if (type == CAST) {
    //         auto &a = spells[pid][idx];
    //         double gain = 0, spent = 0;
    //         for (int t = 0; t < 4; ++t) {
    //             int d = a.delta[t] * max((int)move.get_times(), 1);
    //             if (d > 0) gain += d * ING_W[t];
    //             else spent += -d * ING_W[t];
    //         }
    //         sc += gain * CAST_GAIN_FACTOR
    //             - spent * CAST_SPENT_PENALTY;
    //     }
    //     else if (type == LEARN) {
    //         auto &a = tome_spells[idx];
    //         // you net gain 'tax_count' tier‑0 tokens next turn
    //         // plus any free‐ingredient spells (sum of positive deltas)
    //         double gain = a.tax_count * ING_W[0], spent = 0;
    //         for (int t = 0; t < 4; ++t) {
    //             if (a.delta[t] > 0) gain += a.delta[t] * ING_W[t];
    //             else spent += -a.delta[t] * ING_W[t];
    //         }
    //         sc += gain * LEARN_GAIN_FACTOR
    //             - spent * LEARN_SPENT_PENALTY
    //             - a.tome_index * LEARN_COST_PENALTY;
    //     }
    //     return sc;
    // }

    // void sort_moves(MoveList &moves, const int pid) {
    //     double h_score[moves.size()];
    //     int move_index_to_move_arr_idx[64];
    //     for (int i = 0; i < moves.size(); i++) {
    //         h_score[i] = get_heurestinc_score(moves[i], pid);
    //         move_index_to_move_arr_idx[moves[i].get_index()] = i;
    //     }
    //     sort(moves.begin(), moves.end(), [&](const Move &a, const Move &b) {
    //         return h_score[move_index_to_move_arr_idx[a.get_index()]] 
    //             > h_score[move_index_to_move_arr_idx[b.get_index()]];
    //     });
    // }

    void brew(const Move &move, const int &pid) {
        int idx = move.get_index();
        Action &a = orders[idx];
        p[pid].inv.add(a.cost);
        // p[pid].inv[0] += a.delta[0];
        // p[pid].inv[1] += a.delta[1];
        // p[pid].inv[2] += a.delta[2];
        // p[pid].inv[3] += a.delta[3];
        p[pid].score += a.price;
        p[pid].brewed_count++;
        orders_mask &= ~(uint64_t(1) << idx); // clear the idx th bit (mark idx th order completed)
    }

    int get_brew_bonus(int index_in_queue) {
        if (index_in_queue == 0) {
            if (bonus3_left > 0) {
                return 3;
            } else if (bonus1_left > 0) {
                return 1;
            }
        } else if (index_in_queue == 1 && bonus3_left > 0 && bonus1_left > 0) {
            return 1;
        }
        return 0;
    }

    // avoid putting tax as not simulation OPP, but need to substract need to pay tax for further correctness of ingredients
    void learn(const Move &move, const int &pid) {
        int index = move.get_index(), tax_to_pay = move.tax;
        Action &a = tome_spells[index];

        spells[pid].emplace_back(a);
        tome_spells_mask &= ~(uint64_t(1) << index); // clear the idx th bit (mark idx th tome spell completed)
        // p[pid].inv[0] -= tax_to_pay;
        p[pid].inv.set0(p[pid].inv[0] - tax_to_pay);

        int inv_space = MAX_CAPICITY - p[pid].inv.total(); // (p[pid].inv[0] + p[pid].inv[1] + p[pid].inv[2] + p[pid].inv[3]);
        int tax_available = a.tax_count;

        // p[pid].inv[0] += min(inv_space, tax_available); // take the tax, discard overflow
        p[pid].inv.set0(p[pid].inv[0] + min(inv_space, tax_available));
        p[pid].learn_count++;
    }

    void cast(const Move &move, const int &pid) {
        int idx = move.get_index();
        Action &a = spells[pid][idx];
        int t = move.get_times() == 0 ? 1 : move.get_times();
        // p[pid].inv[0] += a.delta[0] * t;
        // p[pid].inv[1] += a.delta[1] * t;
        // p[pid].inv[2] += a.delta[2] * t;
        // p[pid].inv[3] += a.delta[3] * t;
        Pos temp = a.cost;
        temp.multiply(t);
        p[pid].inv.add(temp);
        spells_mask[pid] &= ~(uint64_t(1) << idx); // clear the idx th bit (mark idx th spell not castable)
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
        undo.inv_old = p[pid].inv;
        undo.score_old = p[pid].score;
        undo.bonus1_old = bonus1_left;
        undo.bonus3_old = bonus3_left;

        if (move_type == REST) {
            undo.type = REST;
            undo.exhausted_mask_old = rest(move, pid);
        } else if (move_type == BREW) {
            undo.type = BREW;
            brew(move, pid);
            int bonus = get_brew_bonus(move.tax);
            p[pid].score += bonus;
            if (bonus == 3) bonus3_left--;
            else if (bonus == 1) bonus1_left--;
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

    Undo_double_move make_double_moves(Move &m1, Move &m2) {
        int type[2] = {m1.get_type(), m2.get_type()};
        int index[2] = {m1.get_index(), m2.get_index()};

        Undo_double_move undo;
        undo.bonus1_old = bonus1_left;
        undo.bonus3_old = bonus3_left;

        int bonus[2] = {0, 0};
        for (int pid = 0; pid <= 1; pid++) {
            undo.idx[pid] = index[pid];
            undo.inv_old[pid] = p[pid].inv;
            undo.score_old[pid] = p[pid].score;

            Move move = (pid == ME ? m1 : m2);
            if (type[pid] == REST) {
                undo.type[pid] = REST;
                undo.exhausted_mask_old[pid] = rest(move, pid);
            } else if (type[pid] == BREW) {
                undo.type[pid] = BREW;
                brew(move, pid);
                bonus[pid] = get_brew_bonus(move.tax);
                p[pid].score += bonus[pid];
            } else if (type[pid] == CAST) {
                undo.type[pid] = CAST;
                cast(move, pid);
            } else if (type[pid] == LEARN) {
                undo.type[pid] = LEARN;
                learn(move, pid);
            } else {
                undo.type[pid] = WAIT;
            }
        }
        if (bonus[ME] == 3 || bonus[OPP] == 3) bonus3_left--;
        if (bonus[ME] == 1 || bonus[OPP] == 1) bonus1_left--;

        turn++;
        return undo;
    }

    void undo_move(const Undo &undo, const int &pid) {
        p[pid].inv = undo.inv_old;
        p[pid].score = undo.score_old;
        bonus1_left = undo.bonus1_old;
        bonus3_left = undo.bonus3_old;
        if (undo.type == BREW) {
            p[pid].brewed_count--;
            orders_mask |= (uint64_t(1) << undo.idx); // set not completed
        } else if (undo.type == LEARN) {
            p[pid].learn_count--;
            spells[pid].pop_back();
            tome_spells_mask |= (uint64_t(1) << undo.idx); // set not completed
        } else if (undo.type == CAST) {
            exhausted_spells[pid]--;
            spells_mask[pid] |= (uint64_t(1) << undo.idx); // set castable true
        } else if (undo.type == REST) {
            spells_mask[pid] = undo.exhausted_mask_old;
        }
        turn--;
    }

    void undo_double_move(const Undo_double_move &undo) {
        bonus1_left = undo.bonus1_old;
        bonus3_left = undo.bonus3_old;
        for (int pid = 0; pid <= 1; pid++) {
            p[pid].inv = undo.inv_old[pid];
            p[pid].score = undo.score_old[pid];
            if (undo.type[pid] == BREW) {
                p[pid].brewed_count--;
                orders_mask |= (uint64_t(1) << undo.idx[pid]); // set not completed
            } else if (undo.type[pid] == LEARN) {
                p[pid].learn_count--;
                spells[pid].pop_back();
                tome_spells_mask |= (uint64_t(1) << undo.idx[pid]); // set not completed
            } else if (undo.type[pid] == CAST) {
                exhausted_spells[pid]--;
                spells_mask[pid] |= (uint64_t(1) << undo.idx[pid]); // set castable true
            } else if (undo.type[pid] == REST) {
                spells_mask[pid] = undo.exhausted_mask_old[pid];
            }
        }
    }

    double get_simple_score(const int pid = ME) {
        return p[pid].score + p[pid].inv.fast_total() - p[pid].inv[0];
    }

    double get_eval(const int depth, const int pid = ME) {
        double eval = 0.0;
        for (int i = 1; i <= 4; i++) {
            eval += i * p[pid].inv[i-1];
        }
        eval += p[pid].score;
        eval += 1.1 * p[pid].brewed_count;
        eval += 0.5 * p[pid].learn_count;

        eval *= depth < MAX_DEPTH ? discount[depth] : 1.0;
        // if (p[pid].brewed_count - global_brew_count[pid] > 0) {
        //     eval += 20;
        // }
        return eval;
    }

    double get_eval_endgame(const int depth = 0) {
        double e1 = get_simple_score(ME), e2 = get_simple_score(OPP);
        return e1 - e2;
        // double eval = 0;
        // if (p[pid].brewed_count == 6 || p[1-pid].brewed_count == 6) {
        //     double e1 = get_simple_score(ME), e2 = get_simple_score(OPP);
        //     if (e1 > e2) return 1e9 + e1 - e2;
        //     else if (e1 < e2) return -1e9 + e1 - e2;
        //     return 0;
        // }

        // if (p[pid].brewed_count == 5 || p[1-pid].brewed_count == 5) {            
        //     eval *= depth < MAX_DEPTH ? discount[depth] * discount[depth] : 1.0;
        // }
        // return eval;
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

unordered_map<string, bool> visited;
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
    // std::pair<double, Move> find_best_move_time_controlled2(Game &game, const int depth, const int &max_depth, const Move last_my_move, const Move last_opp_move) {
    //     nodes_visited++;
    //     if (time_up || depth >= max_depth || 
    //         // last_move.get_type() == BREW || 
    //         game.p[ME].brewed_count >= 6 || game.p[OPP].brewed_count >= 6 || game.turn >= 100) {
    //         return {game.get_eval_both(depth), WAIT_MOVE};
    //     }
    //     double best_eval = game.get_eval_both(depth);
    //     Move best_move = WAIT_MOVE;

    //     MoveList my_moves = game.get_all_moves(ME);
    //     MoveList opp_moves = game.get_all_moves(OPP);

    //     // if avalable move is only WAIT then stop further searching
    //     if (my_moves.size() == 1 && my_moves[0].get_type() == WAIT) return {game.get_eval_both(depth), my_moves[0]};
        
    //     for (int i = 0; i < my_moves.size(); i++) {
    //         if (time_up) return {best_eval, best_move};
    //         Move my_move = my_moves[i];
    //         for (int j = 0; j < opp_moves.size(); j++) {
    //             if (time_up) return {best_eval, best_move};
    //             Move opp_move = opp_moves[j];

    //             Undo_double_move undo = game.make_double_moves(my_move, opp_move);
    //             auto [eval, temp_move] = find_best_move_time_controlled2(game, depth + 1, max_depth, my_move, opp_move);
    //             game.undo_double_move(undo);
    //             if (eval > best_eval) {
    //                 best_eval = eval;
    //                 best_move = my_move;
    //             }
    //         }
    //     }
    //     return {best_eval, best_move};
    // }

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

// struct Beam_node {
//     Game state;         // the game after applying this node’s move sequence
//     Move root_move;     // the very first move from the actual root
//     double eval;        // evaluation of this node
// };
// MoveList get_top_moves(const Game &game, MoveList &moves) {
//     int b_count = 0, l_count = 0, c_count = 0;
//     MoveList top_moves;
//     if (game.turn <= LEARN_MOVE_FORCE_TURNS) {
//         for (Move &move : moves) {
//             if (move.get_type() == LEARN) {
//                 top_moves.push_back(move);
//             }
//         }
//         return moves;
//     }
//     for (Move &move : moves) {
//         if (move.get_type() == BREW && b_count++ < 2) top_moves.push_back(move);
//         if (move.get_type() == CAST && c_count++ < 2) top_moves.push_back(move);
//         if (move.get_type() == LEARN && l_count++ < 2) top_moves.push_back(move);
//     }
//     return top_moves;
// }

// Move beam_search(Game &root, int max_depth, int beam_width, int time_ms) {
//     time_up = false;
//     start_timer(time_ms);
//     vector<Beam_node> beam;
//     auto first_moves = root.get_all_moves(ME);
//     // root.sort_moves(first_moves, ME);
//     // first_moves = get_top_moves(first_moves);

//     visited.clear();
//     visited[root.to_string()] = 1;

//     double best_eval = -1e9;
//     Move best_move = WAIT_MOVE;
//     for (int i = 0; i < first_moves.sz && i < beam_width; ++i) {
//         if (time_up) return best_move;
//         if (root.turn <= LEARN_MOVE_FORCE_TURNS && first_moves[i].get_type() != LEARN) continue;
//         Undo u = root.make_move(first_moves[i], ME);
//         string hash = root.to_string();
//         if (visited.count(hash)) continue;
//         visited[hash] = 1;
//         double eval = root.get_eval(1);
//         if (eval > best_eval) {
//             best_move = first_moves[i];
//             best_eval = eval;
//         }
//         beam.push_back({ root, first_moves[i], eval });
//         root.undo_move(u, ME);
//     }

//     for (int depth = 2; depth <= max_depth; ++depth) {
//         if (time_up) return best_move;
//         vector<Beam_node> next_beam;
//         next_beam.reserve(beam_width);

//         for (auto &node : beam) {
//             if (time_up) return best_move;
//             auto moves = node.state.get_all_moves(ME);
//             // node.state.sort_moves(moves, ME);
//             // moves = get_top_moves(moves);

//             for (int i = 0; i < min<int>(beam_width, moves.sz); ++i) {
//                 if (time_up) return best_move;
//                 if (node.state.turn <= LEARN_MOVE_FORCE_TURNS && moves[i].get_type() != LEARN) continue;
//                 Undo u = node.state.make_move(moves[i], ME);
//                 string hash = node.state.to_string();
//                 if (visited.count(hash)) continue;
//                 visited[hash] = 1;
//                 double eval = node.state.get_eval(depth);
//                 if (eval > best_eval) {
//                     best_move = node.root_move;
//                     best_eval = eval;
//                 }
//                 next_beam.push_back({ node.state, node.root_move, eval });
//                 node.state.undo_move(u, ME);
//             }
//         }
//         if (time_up || next_beam.empty()) return best_move;

//         beam.swap(next_beam);
//         if (beam.size() > beam_width) {
//             sort(beam.begin(), beam.end(), [&](auto &A, auto &B){ return A.eval > B.eval; });
//             beam.resize(beam_width);
//         }
//     }

//     return best_move;
// }


struct BeamNode {
    vector<Move> move_sequence;  // Stores moves instead of full state
    double eval;
    
    Game get_state(const Game& root) const {
        Game g = root;
        for (Move m : move_sequence) g.make_move(m, ME);
        return g;
    }
};

Move optimized_beam_search(Game &root, int max_depth, int beam_width, int time_ms) {
    // Time management
    time_up = false;
    start_timer(time_ms);

    // Memory optimization
    unordered_set<string> visited;
    visited.reserve(beam_width * max_depth * 2);

    // Evaluation cache
    static unordered_map<string, double> eval_cache;
    eval_cache.reserve(beam_width * max_depth);

    // Beam storage using priority queue
    auto cmp = [](const BeamNode& a, const BeamNode& b) { return a.eval < b.eval; };
    priority_queue<BeamNode, vector<BeamNode>, decltype(cmp)> current_beam(cmp);
    vector<BeamNode> next_beam;
    next_beam.reserve(beam_width * 2);

    // Initial beam population
    auto first_moves = root.get_all_moves(ME);
    // partial_sort(first_moves.begin(), first_moves.begin() + min(beam_width, first_moves.size()),
    //            first_moves.end(), [&](Move a, Move b) {
    //                return root.move_heuristic(a) > root.move_heuristic(b);
    //            });

    double best_eval = -1e9;
    Move best_move = WAIT_MOVE;

    for (int i = 0; i < min(beam_width, static_cast<int>(first_moves.size())); ++i) {
        if (time_up) return best_move;
        if (root.turn <= LEARN_MOVE_FORCE_TURNS && first_moves[i].get_type() != LEARN) continue;
        
        Game new_state = root;
        new_state.make_move(first_moves[i], ME);
        string hash = new_state.to_string();
        if (visited.count(hash)) continue;
        visited.insert(hash);

        double eval;
        auto it = eval_cache.find(hash);
        if (it != eval_cache.end()) {
            eval = it->second;
        } else {
            eval = new_state.get_eval(1);
            eval_cache[hash] = eval;
        }

        if (eval > best_eval) {
            best_move = first_moves[i];
            best_eval = eval;
        }

        current_beam.push({{first_moves[i]}, eval});
    }

    // Beam search loop
    for (int depth = 2; depth <= max_depth; ++depth) {
        if (time_up || current_beam.empty()) break;

        next_beam.clear();
        const int TIME_CHECK_INTERVAL = 50;
        int nodes_processed = 0;

        while (!current_beam.empty()) {
            if (nodes_processed++ % TIME_CHECK_INTERVAL == 0 && time_up) 
                return best_move;

            BeamNode node = current_beam.top();
            current_beam.pop();

            Game state = node.get_state(root);
            auto moves = state.get_all_moves(ME);
            
            // Move filtering and ordering
            // moves.erase(remove_if(moves.begin(), moves.end(), 
            //     [&](Move m) { 
            //         return state.turn <= LEARN_MOVE_FORCE_TURNS && m.get_type() != LEARN; 
            //     }), moves.end());
                
            // partial_sort(moves.begin(), moves.begin() + min(beam_width, moves.size()),
            //            moves.end(), [&](Move a, Move b) {
            //                return state.move_heuristic(a) > state.move_heuristic(b);
            //            });

            for (int i = 0; i < min(beam_width, static_cast<int>(moves.size())); ++i) {
                if (state.turn <= LEARN_MOVE_FORCE_TURNS && moves[i].get_type() != LEARN) continue;

                Game new_state = state;
                new_state.make_move(moves[i], ME);
                string hash = new_state.to_string();
                if (visited.count(hash)) continue;
                visited.insert(hash);

                double eval;
                auto it = eval_cache.find(hash);
                if (it != eval_cache.end()) {
                    eval = it->second;
                } else {
                    eval = new_state.get_eval(depth);
                    eval_cache[hash] = eval;
                }

                // Early pruning
                if (eval + 0.1 < best_eval) continue;

                if (eval > best_eval) {
                    best_move = node.move_sequence[0];  // Original root move
                    best_eval = eval;
                }

                BeamNode new_node = node;
                new_node.move_sequence.push_back(moves[i]);
                new_node.eval = eval;
                
                if (next_beam.size() < beam_width) {
                    next_beam.push_back(new_node);
                } else if (eval > next_beam.front().eval) {
                    pop_heap(next_beam.begin(), next_beam.end(), cmp);
                    next_beam.back() = new_node;
                    push_heap(next_beam.begin(), next_beam.end(), cmp);
                }
            }
        }

        for (auto& node : next_beam) {
            current_beam.push(node);
        }
    }

    return best_move;
}


class Endgame {
public:
    // ────────────────────────────────────────────────────────────────────────
    // A simple simultaneous‑move minimax (max–min) with fixed depth
    // ────────────────────────────────────────────────────────────────────────
    /// Recursively evaluate the value of a position after D more plies
    /// where each ply is one simultaneous move (ME and OPP).
    /// Returns the eval from ME’s POV (higher = better for ME).
    double minimax_simultaneous(Game &game, int depth_rem) {
        if (time_up == true
            || depth_rem == 0
            || game.p[ME].brewed_count >= 6
            || game.p[OPP].brewed_count >= 6
            || game.turn >= 100) {
            return game.get_eval_endgame();
        }

        MoveList my_moves  = game.get_all_moves(ME);
        MoveList opp_moves = game.get_all_moves(OPP);

        double best_value = -1e9;
        for (int i = 0; i < my_moves.size(); ++i) {
            if (time_up) return best_value;

            Move my_move = my_moves[i];
            // Assume opponent replies to **minimize** our value
            double worst_for_us = +1e9;
            
            for (int j = 0; j < opp_moves.size(); ++j) {
                if (time_up) return best_value;

                Move opp_move = opp_moves[j];
                auto undo = game.make_double_moves(my_move, opp_move);

                string hash = game.to_string();
                if (visited.count(hash)) continue;
                visited[hash] = 1;

                double v = minimax_simultaneous(game, depth_rem - 1);
                game.undo_double_move(undo);

                worst_for_us = min(worst_for_us, v);
                
                // α–β style cut: if worst_for_us already below best_value, break
                if (worst_for_us <= best_value) break;
            }
            best_value = max(best_value, worst_for_us);
        }

        return best_value;
    }

    /// Top‐level inspector: which **my** move is best under minimax?
    Move find_my_best_simultaneous(Game &game, int depth_rem, int time_ms) {
        time_up = false;
        start_timer(time_ms);
        double best_value = -1e9;
        Move best_move  = WAIT_MOVE;

        MoveList my_moves = game.get_all_moves(ME);
        MoveList opp_moves = game.get_all_moves(OPP);

        visited.clear();

        for (int i = 0; i < my_moves.size(); ++i) {
            if (time_up) return best_move;

            Move my_move = my_moves[i];

            double worst_for_us = +1e9;
            for (int j = 0; j < opp_moves.size(); ++j) {
                if (time_up) return best_move;

                Move opp_move = opp_moves[j];

                auto undo = game.make_double_moves(my_move, opp_move);

                string hash = game.to_string();
                if (visited.count(hash)) continue;
                visited[hash] = 1;

                double v = minimax_simultaneous(game, depth_rem - 1);
                game.undo_double_move(undo);

                worst_for_us = min(worst_for_us, v);
                if (worst_for_us <= best_value) break;
            }

            if (worst_for_us > best_value) {
                best_value = worst_for_us;
                best_move  = my_move;
            }
        }

        return best_move;
    }
};



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

        bool is_opening = game.turn <= LEARN_MOVE_FORCE_TURNS;
        bool is_endgame = game.turn > 90 || game.p[ME].brewed_count >= 4 || game.p[OPP].brewed_count >= 4;

        if (is_opening) {
            cerr << "Opening" << endl;
            // // Beam Search
            // int time_ms = 40;
            // int max_depth = min(100 - game_turn, 30);
            // int max_width = 200;
            // Move move = beam_search(game, max_depth, max_width, time_ms);
            // cerr << "Nodes visited : " << visited.size() << endl;
            // game.output_move(move);

            // Optimized Beam Search
            int time_ms = 40;
            int max_depth = min(100 - game_turn, 15);
            int max_width = 400;
            Move move = optimized_beam_search(game, max_depth, max_width, time_ms);
            game.output_move(move);
        }
        else if (!is_endgame) {
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


            // // Beam Search
            // int time_ms = 20;
            // int max_depth = min(100 - game_turn, 15);
            // int max_width = 200;
            // Move move = beam_search(game, max_depth, max_width, time_ms);
            // cerr << "Nodes visited : " << visited.size() << endl;
            // game.output_move(move);

            // Optimized Beam Search
            int time_ms = 40;
            int max_depth = min(100 - game_turn, 15);
            int max_width = 400;
            Move move = optimized_beam_search(game, max_depth, max_width, time_ms);
            game.output_move(move);
        }
        else if (is_endgame) {
            cerr << "Endgame" << endl;
            int max_depth = 4;
            int time_ms = 45;
            Endgame endgame;
            Move move = endgame.find_my_best_simultaneous(game, max_depth, time_ms);
            game.output_move(move);
        }

        for (int pid = 0; pid <= 1; pid++) {
            prev_score[pid] = game.p[pid].score;
            prev_spells[pid] = game.spells[pid].size();
        }
    }
    return 0;
}














