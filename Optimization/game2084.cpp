#pragma GCC optimize("O3,unroll-loops,fast-math,no-stack-protector")
#pragma GCC target("sse2,avx,avx2,fma")

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <array>
#include <unordered_map>

using namespace std;

// 0,0 is top left

// Types and constants
using BoardType = uint64_t; // 4x4 grid, 16 tiles × 4 bits
using RowType = uint16_t;   // 4 tiles × 4 bits = 16 bits

static constexpr char move_chars[4] = { 'U','D','L','R' };

static constexpr BoardType ROW_MASK = 0xFFFFULL;
static constexpr BoardType COL_MASK = 0x000F000F000F000FULL;

inline long long update_seed(long long seed) {
    return seed * seed % 50515093L;
}

inline BoardType unpack_col(RowType row) {
    BoardType tmp = row;
    return (tmp | (tmp << 12ULL) | (tmp << 24ULL) | (tmp << 36ULL)) & COL_MASK;
}

inline RowType reverse_row(RowType row) {
    return (row >> 12) | ((row >> 4) & 0x00F0)  | ((row << 4) & 0x0F00) | (row << 12);
}

inline RowType get_row(BoardType b, int r) {
    return (b >> (16 * r)) & ROW_MASK;
}
inline void set_row(BoardType &b, int r, RowType row) {
    b &= ~(BoardType(ROW_MASK) << (16 * r));
    b |= (BoardType(row) << (16 * r));
}

inline BoardType transpose(const BoardType &x) {
    BoardType a = (x & 0xF0F00F0FF0F00F0FULL) | ((x & 0x0000F0F00000F0F0ULL) << 12) | ((x & 0x0F0F00000F0F0000ULL) >> 12);
    return (a & 0xFF00FF0000FF00FFULL) | ((a & 0x00FF00FF00000000ULL) >> 24) | ((a & 0x00000000FF00FF00ULL) << 24);
}

static RowType row_left_table[65536];
static RowType row_right_table[65536];
static BoardType col_up_table[65536];
static BoardType col_down_table[65536];
static float heur_score_table[65536];
static float score_table[65536];


struct Board {
    BoardType b;
    Board(BoardType init = 0) : b(init) {}

    inline Board execute_move_0() const { // up
        BoardType ret = 0;
        BoardType t = transpose(b);
        ret |= BoardType(row_left_table[(t >>  0) & ROW_MASK]) << 0;
        ret |= BoardType(row_left_table[(t >> 16) & ROW_MASK]) << 16;
        ret |= BoardType(row_left_table[(t >> 32) & ROW_MASK]) << 32;
        ret |= BoardType(row_left_table[(t >> 48) & ROW_MASK]) << 48;
        return Board(transpose(ret));
    }

    inline Board execute_move_1() const { // down
        BoardType ret = 0;
        BoardType t = transpose(b);
        ret |= BoardType(row_right_table[(t >>  0) & ROW_MASK]) <<  0;
        ret |= BoardType(row_right_table[(t >> 16) & ROW_MASK]) << 16;
        ret |= BoardType(row_right_table[(t >> 32) & ROW_MASK]) << 32;
        ret |= BoardType(row_right_table[(t >> 48) & ROW_MASK]) << 48;
        return Board(transpose(ret));
    }

    inline Board execute_move_2() const { // left
        BoardType ret = 0;
        ret |= BoardType(row_left_table[(b >>  0) & ROW_MASK]) <<  0;
        ret |= BoardType(row_left_table[(b >> 16) & ROW_MASK]) << 16;
        ret |= BoardType(row_left_table[(b >> 32) & ROW_MASK]) << 32;
        ret |= BoardType(row_left_table[(b >> 48) & ROW_MASK]) << 48;
        return Board(ret);
    }

    inline Board execute_move_3() const { // right
        BoardType ret = 0;
        ret |= BoardType(row_right_table[(b >>  0) & ROW_MASK]) <<  0;
        ret |= BoardType(row_right_table[(b >> 16) & ROW_MASK]) << 16;
        ret |= BoardType(row_right_table[(b >> 32) & ROW_MASK]) << 32;
        ret |= BoardType(row_right_table[(b >> 48) & ROW_MASK]) << 48;
        return Board(ret);
    }


    inline Board execute_move(int move) const {
        switch(move) {
        case 0: // up
            return execute_move_0();
        case 1: // down
            return execute_move_1();
        case 2: // left
            return execute_move_2();
        case 3: // right
            return execute_move_3();
        default:
            return ~0ULL;
        }
    }

    inline int at(int r, int c) const {
        return (b >> (4 * (4 * r + c))) & 0xF;
    }
    inline void set(int r, int c, int val) {
        b &= ~(BoardType(0xF) << (4 * (4 * r + c)));
        b |= (BoardType(val & 0xF) << (4 * (4 * r + c)));
    }

    inline int count_empty() const { // board must not be fully empty
        BoardType x = b;
        x |= (x >> 2) & 0x3333333333333333ULL;
        x |= (x >> 1);
        x = ~x & 0x1111111111111111ULL;
        x += x >> 32;
        x += x >> 16;
        x += x >>  8;
        x += x >>  4;
        return int(x & 0xf);
    }

    inline std::pair<int, int> get_spawned_rc(long long seed) const {
        int total = count_empty();
        if (total == 0) return {-1, -1};
        int sp_idx = (int)seed % total;
        int idx = 0;
        for (int c = 0; c < 4; c++) {
            for (int r = 0; r < 4; r++) {
                if (at(r, c) == 0) {
                    if (idx == sp_idx) return {r, c};
                    idx++;
                }
            }
        }
        return {-1, -1};
    }

    inline void spawn(int r, int c, int val) {
        set(r, c, val);
    }
};


inline int tile_value(int expv) { return expv ? (1 << expv) : 0; }


// Heuristic scoring settings
constexpr float SCORE_LOST_PENALTY = 200000.0f;
constexpr float SCORE_MONOTONICITY_POWER = 4.0f;
// const float SCORE_MONOTONICITY_WEIGHT = 47.0f;
constexpr float SCORE_SUM_POWER = 3.5f;
constexpr float SCORE_SUM_WEIGHT = 11.0f;
// const float SCORE_MERGES_WEIGHT = 700.0f;
// const float SCORE_EMPTY_WEIGHT = 270.0f;

constexpr float SCORE_MONOTONICITY_WEIGHT = 60.0f;
constexpr float SCORE_EMPTY_WEIGHT = 350.0f;
constexpr float SCORE_MERGES_WEIGHT = 800.0f;

void init_tables() {
    for (unsigned row = 0; row < 65536; ++row) {
        unsigned line[4] = {
                (row >>  0) & 0xf,
                (row >>  4) & 0xf,
                (row >>  8) & 0xf,
                (row >> 12) & 0xf
        };

        // Score
        float score = 0.0f;
        for (int i = 0; i < 4; ++i) {
            int rank = line[i];
            if (rank >= 2) {
                // the score is the total sum of the tile and all intermediate merged tiles
                score += (rank - 1) * (1 << rank);
            }
        }
        score_table[row] = score;


        // Heuristic score
        float sum = 0;
        int empty = 0;
        int merges = 0;

        int prev = 0;
        int counter = 0;
        for (int i = 0; i < 4; ++i) {
            int rank = line[i];
            sum += pow(rank, SCORE_SUM_POWER);
            if (rank == 0) {
                empty++;
            } else {
                if (prev == rank) {
                    counter++;
                } else if (counter > 0) {
                    merges += 1 + counter;
                    counter = 0;
                }
                prev = rank;
            }
        }
        if (counter > 0) {
            merges += 1 + counter;
        }

        float monotonicity_left = 0;
        float monotonicity_right = 0;
        for (int i = 1; i < 4; ++i) {
            if (line[i-1] > line[i]) {
                monotonicity_left += pow(line[i-1], SCORE_MONOTONICITY_POWER) - pow(line[i], SCORE_MONOTONICITY_POWER);
            } else {
                monotonicity_right += pow(line[i], SCORE_MONOTONICITY_POWER) - pow(line[i-1], SCORE_MONOTONICITY_POWER);
            }
        }

        heur_score_table[row] = SCORE_LOST_PENALTY +
            SCORE_EMPTY_WEIGHT * empty +
            SCORE_MERGES_WEIGHT * merges -
            SCORE_MONOTONICITY_WEIGHT * std::min(monotonicity_left, monotonicity_right) -
            SCORE_SUM_WEIGHT * sum;

        // execute a move to the left
        int temp[4], idx = 0;
        for (int i = 0; i < 4; i++) {
            if (line[i] != 0) {
                temp[idx++] = line[i];
            }
        }
        for (int i = 0, j = 0; i < 4; i++) {
            if (j >= idx) line[i] = 0;
            else if (j+1 < idx && temp[j] == temp[j+1]) {
                line[i] = temp[j] + 1;
                j += 2;
            } else {
                line[i] = temp[j];
                j++;
            }
        }

        RowType result = (line[0] <<  0) | (line[1] <<  4) | (line[2] <<  8) | (line[3] << 12);
        RowType rev_result = reverse_row(result);
        unsigned rev_row = reverse_row(row);

        row_left_table[row] = result;
        row_right_table[rev_row] = rev_result;
    }
}

// double positional_score(const Board &b) {
//     static const double POS_W[4][4] = {
//         { 32768.0, 16384.0, 4096.0, 1024.0 },
//         { 512.0,   256.0,   128.0,  64.0   },
//         { 32.0,    16.0,    8.0,    4.0    },
//         { 2.0,     1.0,     0.5,    0.25   }
//     };

//     double pos_sum = 0.0;
//     for (int r=0;r<4;++r) for (int c=0;c<4;++c) {
//         int v = b.at(r,c);
//         double tv = v ? double(1<<v) : 0.0;
//         pos_sum += tv * POS_W[r][c];
//     }
//     return pos_sum;
// }

inline double eval_helper(const BoardType b) {
    return heur_score_table[(b >>  0) & ROW_MASK] +
           heur_score_table[(b >> 16) & ROW_MASK] +
           heur_score_table[(b >> 32) & ROW_MASK] +
           heur_score_table[(b >> 48) & ROW_MASK];
}

inline double evaluate(const Board &board) {
    return eval_helper(board.b) + eval_helper(transpose(board.b));
}


// BeamSearch
class BeamSearch {
public:
    struct Node {
        Board board;
        long long seed;
        std::string moves;
        double score;
    };

    BeamSearch(int beam_width = 300, int max_steps = 1000) : W(beam_width), max_steps(max_steps) {}

    std::string search(const Board &start, long long seed) {
        std::vector<Node> beam;
        beam.reserve(W);
        beam.push_back({ start, seed, std::string(), evaluate(start) });

        for (int step = 0; step < max_steps; ++step) {
            std::vector<Node> candidates;
            candidates.reserve(beam.size() * 4);

            // used to keep only the best node for each (board,seed) to avoid duplicates in next beam
            std::unordered_map<uint64_t, double> best_score_for_state;
            best_score_for_state.reserve(beam.size() * 4);

            for (const Node &node : beam) {
                for (int mv = 0; mv < 4; ++mv) {
                    Board nb = node.board.execute_move(mv);
                    if (nb.b == node.board.b) continue;

                    auto [r, c] = nb.get_spawned_rc(node.seed);
                    long long new_seed = node.seed;
                    if (r != -1) {
                        nb.spawn(r, c, ((node.seed & 0x10) == 0) ? 1 : 2);
                        new_seed = update_seed(node.seed);
                    } else {
                        // no spawn => seed unchanged => to matche simulation behavior
                    }

                    uint64_t key = hash_board_seed(nb.b, new_seed);

                    double sc = evaluate(nb);

                    // if we already saw this exact (board,seed) choose the best-scoring one
                    auto it = best_score_for_state.find(key);
                    if (it == best_score_for_state.end() || sc > it->second) {
                        best_score_for_state[key] = sc;
                        candidates.push_back({ nb, new_seed, node.moves + move_chars[mv], sc });
                    }
                }
            }

            if (candidates.empty()) break;

            // Keep top-W candidates
            if ((int)candidates.size() > W) {
                std::partial_sort(candidates.begin(), candidates.begin() + W, candidates.end(), 
                    [](const Node &a, const Node &b){ return a.score > b.score; });
                candidates.resize(W);
            }

            beam = std::move(candidates);
        }

        if (beam.empty()) return std::string();
        auto best_it = std::max_element(beam.begin(), beam.end(), 
            [](const Node &a, const Node &b){ return a.score < b.score; });
        if (best_it == beam.end()) return std::string();
        return best_it->moves;
    }

private:
    int W;
    int max_steps;

    static inline uint64_t hash_board_seed(BoardType b, long long seed) {
        uint64_t s = (uint64_t)seed;
        // mix board and seed (cheap and good enough for 64-bit keys)
        uint64_t h = b;
        h ^= (s + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2));
        // final xorshift
        h ^= h >> 33;
        h *= 0xff51afd7ed558ccdULL;
        h ^= h >> 33;
        return h;
    }
};



struct BeamParams {
    int width;
    int max_steps;
    int max_move_len;
};

class AdaptiveBeamSearch {
    double expansions_per_sec;
    double beam_ratio; // controls width vs depth
public:
    AdaptiveBeamSearch(double exp_per_sec = 250000, double ratio = 0.5)
        : expansions_per_sec(exp_per_sec), beam_ratio(ratio) {}

    inline BeamParams getParams(double ms_budget) {
        // total expansions we can afford
        long long budget_expansions = (long long)(expansions_per_sec * (ms_budget / 1000.0));

        // distribute between width and steps
        // width ~ sqrt(budget), steps ~ sqrt(budget)
        int W = std::max(50, (int)(sqrt(budget_expansions) * beam_ratio));
        int S = std::max(10, (int)(sqrt(budget_expansions) * (1.0 - beam_ratio) + 5));

        BeamParams bp;
        bp.width = W;
        bp.max_steps = S;
        bp.max_move_len = (80*S) / 100; // keep some difference between steps and move length
        return bp;
    }
};


void read_board(Board &b) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int x;
            cin >> x;
            b.set(i, j, x == 0 ? 0 : log2(x));
        }
    }
}

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    init_tables();

    Board b;

    int turn = 1;
    long long seed;
    int score;

    AdaptiveBeamSearch tuner(1e6, 0.6); // measured expansions/sec and beam ratio
    

    // cerr << params1.width << " " << params1.max_steps << " " << params1.max_move_len << endl;
    // cerr << params2.width << " " << params2.max_steps << " " << params2.max_move_len << endl;

    while (1) {
        cin >> seed; cin.ignore();
        cin >> score; cin.ignore();
        read_board(b);

        if (turn == 1) {
            BeamParams params = tuner.getParams(400);
            BeamSearch bs(params.width, params.max_steps);
            std::string seq = bs.search(b, seed);
            cout << seq.substr(0, min({(int)seq.size(), params.max_move_len})) << "-" << endl;
        } else {
            BeamParams params = tuner.getParams(30);
            BeamSearch bs(params.width, params.max_steps);
            std::string seq = bs.search(b, seed);
            cout << seq.substr(0, min({(int)seq.size(), params.max_move_len})) << "-" << endl;
        }

        turn++;
    }
}



