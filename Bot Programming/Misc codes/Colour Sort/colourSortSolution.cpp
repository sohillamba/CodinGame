#include <iostream>
#include <vector>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <queue>

#include <array>
#include <cstddef>
#include <functional>

using namespace std;

// static constexpr size_t MAX_TUBES = 3;
static constexpr size_t MAX_TUBES = 14;
static constexpr size_t H = 4;

using Key = std::array<uint8_t, MAX_TUBES * H>;

struct KeyHash {
    size_t operator()(Key const &k) const noexcept {
        size_t h = 146527; 
        for (auto byte : k) {
            h = (h * 1315423911u) ^ byte;
        }
        return h;
    }
};

struct KeyEq {
    bool operator()(Key const &a, Key const &b) const noexcept {
        return a == b;
    }
};

enum Color {
    red = 1, purple = 2, pink = 3, blue = 4, grey = 5,
    sky = 6, lightGreen = 7, green = 8, orange = 9, yellow = 10, 
    lightPink = 11, white = 12
};

class Tube {
public:
    uint8_t sz = 0;
    vector<pair<Color, uint8_t>> colors;

    Tube (vector<Color> cs) {
        reverse(cs.begin(), cs.end());
        sz = cs.size();
        for (Color &c : cs) {
            if (colors.empty() || colors.back().first != c) {
                colors.push_back({c, 1});
            } else {
                colors.back().second++;
            }
        }
    }

    uint8_t size() {
        return sz;
    }

    bool empty() {
        return sz == 0;
    }

    pair<Color, uint8_t>& back() {
        if (sz == 0) {
            cout << "sz is zero\n";
            exit(0);
        }
        if (colors.empty()) {
            cout << "colors is empty\n";
            exit(0);
        }
        return *(colors.begin() + (colors.size() - 1));
    }

    void add(const uint8_t &count) {
        sz += count;
        colors.back().second += count;
    }

    void substract(const uint8_t &count) {
        sz -= count;
        colors.back().second -= count;
        if (colors.back().second == 0) {
            colors.pop_back();
        }
    }

    void add(pair<Color, uint8_t> c) {
        sz += c.second;
        colors.emplace_back(c);
    }

    void remove() {
        sz -= colors.back().second;
        colors.pop_back();
    }

    void print(int idx) {
        cout << idx << ": ";
        for (auto &&[a, b] : colors) {
            // for (int i = 0; i < b; i++) {
            //     cout << a << "\t";
            // }
            cout << 0 + a << " " << 0 + b << ",\t";
        }
        cout << "\n";
    }
};

class Move {
public:
    uint8_t data;

    Move (uint8_t from, uint8_t to) {
        data = from;
        data <<= 4;
        data |= to;
    }

    void set (uint8_t from, uint8_t to) {
        data = from;
        data <<= 4;
        data |= to;
    }

    uint8_t getFrom() {
        return (data >> 4) & 0b1111;
    }

    uint8_t getTo() {
        return data & 0b1111;
    }
};

class Game {
public:
    const static uint8_t n = MAX_TUBES;
    vector<Tube> tubes;

    uint8_t remaining;
    vector<bool> isSolved;
    unordered_set<uint8_t> available;

    Game() {
        remaining = n - 2;
        isSolved = vector<bool> (n, false);
        for (uint8_t i = 0; i < n; i++) {
            available.insert(i);
        }

        tubes.push_back(Tube({Color::red, Color::purple, Color::pink, Color::red}));
        tubes.push_back(Tube({Color::blue, Color::grey, Color::grey, Color::purple}));
        tubes.push_back(Tube({Color::grey, Color::sky, Color::red, Color::blue}));
        tubes.push_back(Tube({Color::lightGreen, Color::green, Color::orange, Color::lightGreen}));

        tubes.push_back(Tube({Color::pink, Color::yellow, Color::sky, Color::purple}));
        tubes.push_back(Tube({Color::lightPink, Color::lightGreen, Color::pink, Color::yellow}));
        tubes.push_back(Tube({Color::pink, Color::lightPink, Color::blue, Color::purple}));
        tubes.push_back(Tube({Color::red, Color::sky, Color::white, Color::yellow}));

        tubes.push_back(Tube({Color::lightPink, Color::orange, Color::grey, Color::yellow}));
        tubes.push_back(Tube({Color::white, Color::sky, Color::lightGreen, Color::white}));
        tubes.push_back(Tube({Color::orange, Color::green, Color::blue, Color::orange}));
        tubes.push_back(Tube({Color::green, Color::lightPink, Color::green, Color::white}));
        
        tubes.push_back(Tube({}));
        tubes.push_back(Tube({}));

        for (uint8_t i = 0; i < n; i++) {
            if (!tubes[i].empty() && tubes[i].back().second == H) {
                remaining--;
                isSolved[i] = true;
                available.erase(i);
            }
        }
    }

    // Game() {
    //     remaining = n - 1;
    //     isSolved = vector<bool> (n, false);
    //     for (uint8_t i = 0; i < n; i++) {
    //         available.insert(i);
    //     }

    //     tubes.push_back(Tube({Color::yellow, Color::purple, Color::yellow, Color::purple}));
    //     tubes.push_back(Tube({Color::purple, Color::yellow, Color::purple, Color::yellow}));
        
    //     tubes.push_back(Tube({}));

    //     for (uint8_t i = 0; i < n; i++) {
    //         if (!tubes[i].empty() && tubes[i].back().second == H) {
    //             remaining--;
    //             isSolved[i] = true;
    //             available.erase(i);
    //         }
    //     }
    // }

    bool canMove(Move &move) {
        uint8_t f = move.getFrom(), t = move.getTo();

        // False : from empty or to is full
        if (tubes[f].empty() || tubes[t].size() == H) return false;

        uint8_t fCount = tubes[f].back().second;
        // False : from is fully sorted
        if (fCount == H) return false;

        // to is empty -> False if from has only one color
        if (tubes[t].empty()) return fCount != tubes[f].size();

        // False : color mismatch
        if (tubes[t].back().first != tubes[f].back().first) return false;

        // False : overflow (not allowing partial moving as it is inefficient)
        return fCount + tubes[t].size() <= H;
    }

    uint8_t makeMove(Move &move) {
        uint8_t f = move.getFrom(), t = move.getTo();
        uint8_t toMove = tubes[f].back().second;
        if (tubes[t].empty()) {
            tubes[t].add(tubes[f].back());
        } else {
            tubes[t].add(toMove);
        }
        tubes[f].remove();
        if (tubes[t].back().second == H) {
            remaining--;
            available.erase(t);
            isSolved[t] = true;
        }
        return toMove;
    }

    void undoMove(Move &move, uint8_t &count) {
        uint8_t f = move.getFrom(), t = move.getTo();
        if (tubes[t].back().second == H) {
            remaining++;
            available.insert(t);
            isSolved[t] = false;
        }
        tubes[f].add({tubes[t].back().first, count});
        if (tubes[f].back().second == H) {
            remaining--;
            available.erase(f);
            isSolved[f] = true;
        }
        tubes[t].substract(count);
    }

    bool isGameSolved() {
        return remaining == 0;
    }

    Key getKey() {
        Key k{};
        size_t idx = 0;
        for (size_t t = 0; t < tubes.size(); ++t) {
            size_t start = t * H;
            for (auto &run : tubes[t].colors) {
                for (uint8_t cnt = 0; cnt < run.second; ++cnt) {
                    k[idx++] = static_cast<uint8_t>(run.first);
                }
            }
            while (idx < start + H) {
                k[idx++] = 0;
            }
        }
        return k;
    }

    string getKeyString() {
        vector<string> all;
        for (Tube &tube : tubes) {
            string cur = "";
            for (auto &&[color, count] : tube.colors) {
                for (uint8_t i = 0; i < count; i++) {
                    cur += 'a' + (0 + color);
                }
            }
            all.emplace_back(cur);
        }
        sort(all.begin(), all.end());
        string res = "";
        for (string &s : all) {
            res += s;
            res += ',';
        }
        if (!res.empty()) res.pop_back();
        return res;
    }

    void print() {
        for (int i = 0; i < n; i++) {
            tubes[i].print(i);
        }
        cout << "\n";
    }
};

Game game;
vector<Move> moves, curMoves;

unordered_set<Key, KeyHash, KeyEq> seen;
unordered_set<string> visited;              // faster in dfs, ignores symetricall states i.e. only tube order is different

bool solve_dfs(int depth = 1000000) {
    if (game.isGameSolved()) return true;
    if (depth <= 0) return false;

    Key key = game.getKey();
    if (!seen.insert(key).second) {
        return false;
    }

    // string key = game.getKeyString();
    // if (visited.find(key) != visited.end()) return false;
    // visited.insert(key);

    Move move(0, 0);
    uint8_t count;

    unordered_set<uint8_t> ij = game.available;
    for (uint8_t i : ij) {
        if (game.isSolved[i] || game.tubes[i].empty()) continue;
        bool emptyFound = false;

        for (uint8_t j : ij) {
            if (game.isSolved[j]) continue;
            if (i == j) continue;
            if (emptyFound) continue;
            if (game.tubes[j].empty()) emptyFound = true;
            
            move.set(i, j);
            if (game.canMove(move)) {
                count = game.makeMove(move);
                if (solve_dfs(depth - 1)) {
                    game.undoMove(move, count);
                    moves.push_back(move);
                    return true;
                }
                game.undoMove(move, count);
            }
        }
    }
    return false;
}



struct State {
    Game game;
    vector<Move> path;
};

void solve_bfs() {
    seen.clear();
    moves.clear();

    queue<State> q;
    q.push({game, {}});

    int depth = 41;

    while (!q.empty() && depth > 0) {
        depth--;
        int sz = q.size();
        while (sz--) {
            State cur = q.front();
            q.pop();

            if (cur.game.isGameSolved()) {
                moves = cur.path;
                return;
            }

            // Key key = cur.game.getKey();
            // if (!seen.insert(key).second) continue;

            string key = cur.game.getKeyString();
            if (visited.find(key) != visited.end()) continue;
            visited.insert(key);

            unordered_set<uint8_t> ij = cur.game.available;

            for (uint8_t i : ij) {
                if (cur.game.isSolved[i] || cur.game.tubes[i].empty()) continue;
                bool emptyFound = false;

                for (uint8_t j : ij) {
                    if (cur.game.isSolved[j]) continue;
                    if (i == j) continue;
                    if (emptyFound) continue;
                    if (cur.game.tubes[j].empty()) emptyFound = true;

                    Move move(i, j);
                    if (cur.game.canMove(move)) {
                        Game newGame = cur.game;
                        uint8_t count = newGame.makeMove(move);

                        vector<Move> newPath = cur.path;
                        newPath.push_back(move);

                        q.push({newGame, newPath});
                    }
                }
            }
        }
    }
    cout << "depth: " << depth << "\n";
}



int main() {
    game = Game();
    seen.clear();
    visited.clear();
    moves.clear();
    curMoves.clear();

    solve_dfs();
    reverse(moves.begin(), moves.end());

    for (auto &move : moves) {
        cout << move.getFrom() + 1 << " " << move.getTo() + 1 << "\n";
    }

    cout << "\nDone!!\n";
    cout << "seen: " << seen.size() << "\n";
    cout << "vis:  " << visited.size() << "\n";
    return 0;
}



