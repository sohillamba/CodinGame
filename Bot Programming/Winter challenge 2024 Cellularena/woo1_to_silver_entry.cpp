#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <queue>
#include <cstring>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <climits>

using namespace std;

const vector<string> DIR = {"E", "W", "N", "S"};
const vector<string> OPP_DIR = {"W", "E", "S", "N"};
const vector<pair<int, int>> dirs = {{1, 0}, {-1, 0}, {0, -1}, {0, 1}};
const unordered_map<string, int> dir_idx = {{"E", 0}, {"W", 1}, {"N", 2}, {"S", 3}};
// const unordered_map<string, string> oppsite_dir = {{"E", "W"}, {"W", "E"}, {"N", "S"}, {"S", "N"}};

const int ME = 1, HE = 0;

const string WALL = "WALL";
const string ROOT = "ROOT";
const string BASIC = "BASIC";
const string HARVESTER = "HARVESTER";
const string TENTACLE = "TENTACLE";
const string SPORER = "SPORER";

int W, H;

inline bool isValid(int x, int y) {
    return x >= 0 && x < W && y >= 0 && y < H;
}

inline int manhattan(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

class Proteins {
public:
    int a, b, c, d;
    Proteins() {
        a = b = c = d = 0;
    }
    Proteins(int _a, int _b, int _c, int _d) {
        a = _a; b = _b; c = _c; d = _d;
    }
    void read() {
        cin >> a >> b >> c >> d;
        // cerr << a << " " << b << " " << c << " " << d << endl;
    }
};

const unordered_map<string, Proteins> ORGAN_COSTS = {
    {BASIC, {1,0,0,0}},
    {HARVESTER, {0,0,1,1}},
    {TENTACLE, {0,1,1,0}},
    {SPORER, {0,1,0,1}},
    {ROOT, {1,1,1,1}}
};

class Entity {
public:
    // par_id of root is 0
    // root_id of root is its id i.e. id = root_id for roots
    int id, par_id, root_id;
    int x, y;
    int owner = -1;             // 1 : me, 0 : enemy, -1 : not organ
    string type = "";           // WALL, ROOT, BASIC, HARVESTER, TENTACLE, SPORER, A, B, C, D
    string dir;                 // N, W, S, E (N: up, W: left, S: down, E: right)

    Entity() : owner(-1), type("") {}

    void read() {
        cin >> x >> y >> type >> owner >> id >> dir >> par_id >> root_id;
        // if (type != WALL) {
        //     cerr << x << " " << y << " " << type << " " << owner << " " << id << " " << dir << " " << par_id << " " << root_id << endl;
        // }
    }

    bool is_protein() {
        return is_a() || is_b() || is_c() || is_d();
    }
    bool is_a() {
        return type == "A";
    }
    bool is_b() {
        return type == "B";
    }
    bool is_c() {
        return type == "C";
    }
    bool is_d() {
        return type == "D";
    }

    bool is_mine() {
        return owner == ME;
    }
    bool is_enemy() {
        return owner == HE;
    }

    bool is_empty() {
        return type == "";
    }
    void reset() {
        type = "";
    }
    bool can_move_over() {
        return type == "" || is_protein(); 
    }

    void print() {
        cerr << "x: " << x << " ";
        cerr << "y: " << y << " ";
        cerr << "id: " << id << " ";
        cerr << "owner: " << owner << " ";
        cerr << "type: " << type << " ";
        cerr << endl;
    }
};

class Player {
public:
    int id;
    Proteins p;
    vector<vector<Entity>> organisms;
    unordered_map<int, int> root_id_to_index; // root it to index in organisms vector
    unordered_map<int, vector<int>> children_ids; // children ids of an organ with id i
    unordered_map<int, Entity> id_to_entity;

    Player(int id) : id(id) {}

    void input_protein() {
        p.read();
    }

    void setup_organisms(vector<Entity> &all_organs) {
        for (auto &entity : all_organs) {
            if (root_id_to_index.find(entity.root_id) == root_id_to_index.end()) {
                root_id_to_index[entity.root_id] = organisms.size();
                vector<Entity> organism;
                organism.emplace_back(entity);
                organisms.emplace_back(organism);
            } else {
                organisms[root_id_to_index[entity.root_id]].emplace_back(entity);
            }
            if (entity.par_id != 0) {
                children_ids[entity.par_id].push_back(entity.id);
            }
            id_to_entity[entity.id] = entity;
        }
    }

    void clear() {
        organisms.clear();
        root_id_to_index.clear();
    }

    // bool can_grow_type(string &type) {
    //     if (type == BASIC) return can_grow_basic();
    //     if (type == HARVESTER) return can_grow_harvestor();
    //     if (type == TENTACLE) return can_grow_tentacle();
    //     if (type == SPORER) return can_grow_sporer();
    //     if (type == ROOT) return can_spore_root();
    //     return false;
    // }
    bool can_grow_type(const string& type) const {
        auto& cost = ORGAN_COSTS.at(type);
        return p.a >= cost.a && p.b >= cost.b && p.c >= cost.c && p.d >= cost.d;
    }
    bool can_grow_basic() {
        return p.a > 0;
    }
    bool can_grow_harvestor() {
        return p.c > 0 && p.d > 0;
    }
    bool can_grow_tentacle() {
        return p.b > 0 && p.c > 0;
    }
    bool can_grow_sporer() {
        return p.b > 0 && p.d > 0;
    }
    bool can_spore_root() {
        return p.a > 0 && p.b > 0 && p.c > 0 && p.d > 0;
    }

    // void grow_type(const string& type) const {
    //     auto& cost = ORGAN_COSTS.at(type);
    //     p.a -= cost.a;
    //     p.b -= cost.b;
    //     p.c -= cost.c;
    //     p.d -= cost.d;
    // }
    void grow_basic() {
        p.a -= 1;
    }
    void grow_harvestor() {
        p.c -= 1; p.d -= 1;
    }
    void grow_tentacle() {
        p.b -= 1; p.c -= 1;
    }
    void grow_sporer() {
        p.b -= 1; p.d -= 1;
    }
    void spore_root() {
        p.a -= 1; p.b -= 1; p.c -= 1; p.d -= 1;
    }
};


class Game {
public:
    int turns;
    Player me, he;
    vector<vector<Entity>> grid;
    vector<vector<int>> distance; // paths dont include organs except start and end
    vector<vector<bool>> harvested_by_me;
    vector<vector<bool>> harvested_by_opp;

    Game() : me(ME), he(HE) {
        turns = 0;
        grid = vector<vector<Entity>> (W, vector<Entity>(H));
        harvested_by_me = vector<vector<bool>>(W, vector<bool>(H, false));
        harvested_by_opp = vector<vector<bool>>(W, vector<bool>(H, false));
    }

    void compute_harvested() {
        harvested_by_me = vector<vector<bool>>(W, vector<bool>(H, false));
        harvested_by_opp = vector<vector<bool>>(W, vector<bool>(H, false));
        
        for (int x = 0; x < W; x++) {
            for (int y = 0; y < H; y++) {
                if (grid[x][y].type == HARVESTER) {
                    auto it = dir_idx.find(grid[x][y].dir);
                    if (it == dir_idx.end()) continue;
                    
                    auto [dx, dy] = dirs[it->second];
                    int tx = x + dx, ty = y + dy;
                    if (isValid(tx, ty)) {
                        if (grid[x][y].owner == ME) {
                            harvested_by_me[tx][ty] = grid[tx][ty].is_protein();
                        } else {
                            harvested_by_opp[tx][ty] = grid[tx][ty].is_protein();
                        }
                    }
                }
            }
        }
    }

    void set_dis_from_enemy() {
        distance = vector<vector<int>> (W, vector<int> (H, (W + 5) * (H + 5)));
        vector<vector<int>> vis(W, vector<int> (H, 0));
        queue<pair<int, int>> q;
        for (auto &organism : he.organisms) {
            for (auto &o : organism) {
                q.push({o.x, o.y});
                vis[o.x][o.y] = 1;
            }
        }
        int dis = 1;
        while (!q.empty()) {
            int sz = q.size();
            while (sz--) {
                auto [x, y] = q.front(); q.pop();
                for (auto [dx, dy] : dirs) {
                    int x2 = x + dx, y2 = y + dy;
                    if (isValid(x2, y2) && !vis[x2][y2]) {
                        vis[x2][y2] = 1;
                        distance[x2][y2] = dis;
                        if (grid[x2][y2].owner == -1 && grid[x2][y2].type != WALL) {
                            q.push({x2, y2});
                        }
                    }
                }
            }
            dis++;
        }
    }

    vector<Entity> get_best_moves_list() {
        vector<Entity> best_moves;
        unordered_set<int> processed_roots;
        const vector<vector<Entity>> grid_backup = grid;
        const Player me_backup = me;

        // process one move per root (per organism)
        for (int attempt = 0; attempt < me.organisms.size(); attempt++) {
            double best_score = -1e9;
            int best_root_id = -1;
            Entity best_move;
            
            // Consider all roots not yet processed, and take the best move
            for (auto &organism : me.organisms) {
                if (organism.empty()) continue;
                int root_id = organism[0].root_id;
                if (processed_roots.count(root_id)) continue;

                // Get moves for this root only
                vector<Entity> moves = get_all_moves(me, root_id);
                vector<Entity> spore_moves = get_spore_root_moves(me, root_id);
                moves.insert(moves.end(), spore_moves.begin(), spore_moves.end());

                if (moves.empty()) continue;

                // Find best move for this root
                for (auto &move : moves) {
                    move.root_id = root_id;
                    double score = get_score(move);
                    if (score > best_score) {
                        best_score = score;
                        best_move = move;
                        best_root_id = root_id;
                    }
                }
            }

            if (best_root_id == -1) break;  // No valid moves left

            // Save best move for this root
            best_moves.emplace_back(best_move);
            processed_roots.insert(best_root_id);
            make_temp_move(best_move, me);
        }

        // random moves (can be WAIT) for remaining roots
        for (auto &organism : me.organisms) {
            int root_id = organism[0].root_id;
            if (processed_roots.count(root_id)) continue;
            Entity move = get_random_move(root_id);
            best_moves.emplace_back(move);
            processed_roots.insert(root_id);
        }

        // Restore original state
        me = me_backup;
        grid = grid_backup;
        return best_moves;
    }

    void make_temp_move(Entity &move, Player &p) {
        // grid[move.x][move.y].type = WALL;
        grid[move.x][move.y].type = move.type;
        grid[move.x][move.y].owner = -1;
        if (move.type == BASIC) {
            p.grow_basic();
        } else if (move.type == HARVESTER) {
            p.grow_harvestor();
        } else if (move.type == TENTACLE) {
            p.grow_tentacle();
        } else if (move.type == SPORER) {
            p.grow_sporer();
        } else if (move.type == ROOT) {
            p.spore_root();
        }
    }

    // get all moves (not new root moves) only for root_id
    vector<Entity> get_all_moves(Player &p, const int &root_id) {
        map<pair<int, int>, int> moves;
        auto &organism = me.organisms[me.root_id_to_index[root_id]];
        for (auto &e : organism) {
            for (auto [dx, dy] : dirs) {
                int x = e.x + dx, y = e.y + dy;
                if (isValid(x, y) && grid[x][y].can_move_over()) {
                    moves[{x, y}] = e.id;
                }
            }
        }
        vector<Entity> entities;
        Entity e;
        for (auto [xy, par_id] : moves) {
            e.x = xy.first;
            e.y = xy.second;
            e.par_id = par_id;
            if (p.can_grow_basic()) {
                e.type = BASIC;
                entities.emplace_back(e);
            }
            for (string d : DIR) {
                e.dir = d;
                if (p.can_grow_harvestor()) {
                    e.type = HARVESTER;
                    entities.emplace_back(e);
                }
                if (p.can_grow_tentacle()) {
                    e.type = TENTACLE;
                    entities.emplace_back(e);
                }
                if (p.can_grow_sporer()) {
                    e.type = SPORER;
                    entities.emplace_back(e);
                }
            }
        }
        return entities;
    }

    // only considers possible roots which are at min distance MIN_DISTANCE from the sporer
    vector<Entity> get_spore_root_moves(Player &p, const int &root_id) {
        if (!p.can_spore_root()) return {};

        const static int MIN_DISTANCE = 3;
        map<pair<int, int>, int> moves; // coordinates with id of organ from which creating this new organ
        auto &organism = me.organisms[me.root_id_to_index[root_id]];
        
        for (auto &e : organism) {
            if (e.type != SPORER) continue;
            auto [dx, dy] = dirs[dir_idx.at(e.dir)];
            
            // Prioritize longest possible paths
            for (int dist = 1; ; dist++) {
                int x2 = e.x + dx*dist, y2 = e.y + dy*dist;
                if (!isValid(x2, y2) || !grid[x2][y2].can_move_over()) break;
                // Only consider endpoints with expansion potential
                if (dist >= MIN_DISTANCE) moves[{x2, y2}] = e.id;  
            }
        }
        vector<Entity> entities;
        for (auto [pos, par_id] : moves) {
            Entity e;
            e.type = ROOT;
            e.x = pos.first;
            e.y = pos.second;
            e.par_id = par_id;
            entities.emplace_back(e);
        }
        return entities;
    }

    // random move, WAIT for now
    Entity get_random_move(const int &root_id) {
        Entity e;
        e.type = "WAIT";
        return e;
    }

    int controlled_by_my_tentacle(int x, int y) const;
    int controlled_by_enemy_tentacle(int x, int y) const;
    int controlled_by_player_tentacle(int x, int y, int p_id) const;

    int destroyed_opponent_organs_if_here(int x, int y, const string &dir) const;

    double get_protein_need(char protein_type) const;

    double get_score(const Entity &e);
    double score_basic(const Entity& e);
    double score_harvester(const Entity& e);
    double score_tentacle(const Entity& e);
    double score_sporer(const Entity& e);
    double score_root(const Entity& e);
};

int Game::controlled_by_my_tentacle(int x, int y) const {
    return controlled_by_player_tentacle(x, y, ME);
}
int Game::controlled_by_enemy_tentacle(int x, int y) const {
    return controlled_by_player_tentacle(x, y, HE);
}
int Game::controlled_by_player_tentacle(int x, int y, int p_id) const {
    int count = 0;
    for (int d = 0; d < 4; d++) {
        int x2 = x + dirs[d].first, y2 = y + dirs[d].second;
        if (isValid(x2, y2)) {
            const Entity& o = grid[x2][y2];
            if (o.type == TENTACLE && o.owner == p_id && o.dir == OPP_DIR[d]) {
                count++;
            }
        }
    }
    return count;
}

int count_children(const Player &p, int id) {
    int ans = 0;
    if (p.children_ids.find(id) != p.children_ids.end()) {
        ans = p.children_ids.at(id).size();
        for (int ch_id : p.children_ids.at(id)) {
            ans += count_children(p, ch_id);
        }
    }
    return ans;
}

// Count how many opponent organs would be “destroyed” by placing here.
int Game::destroyed_opponent_organs_if_here(int x, int y, const string &dir) const {
    auto [dx, dy] = dirs[dir_idx.at(dir)];
    int x2 = x + dx, y2 = y + dy;
    if (isValid(x2, y2) && grid[x2][y2].owner == HE) {
        int ans = count_children(he, grid[x2][y2].id);
        cerr << "count_children: " << ans << endl;
        return ans;
    }
    return 0;
}


double Game::get_score(const Entity &e) {
    if (e.type == TENTACLE) return score_tentacle(e);
    if (e.type == HARVESTER) return score_harvester(e);
    if (e.type == SPORER) return score_sporer(e);
    if (e.type == BASIC) return score_basic(e);
    if (e.type == ROOT) return score_root(e);
    return 0.0;
}

double Game::score_tentacle(const Entity& e) {
    double score = 0.0;
    int x = e.x, y = e.y;
    
    // Attack potential (60% weight)
    auto [dx, dy] = dirs[dir_idx.at(e.dir)];
    int tx = x + dx, ty = y + dy;
    if (isValid(tx, ty)) {
        Entity target = grid[tx][ty];
        if (target.owner == HE) {
            int kills = 1 + count_children(he, target.id);
            score += kills * 12.0;  // Increased from 5
            if (target.type == ROOT) score += 25.0;  // Massive bonus for killing roots
        }
    }
    
    // Proximity bonus (30% weight)
    int d_to_enemy = distance[x][y];
    score += (d_to_enemy > 0) ? 25.0 / d_to_enemy : 25.0;
    
    // Defense penalty (10% weight)
    if (controlled_by_enemy_tentacle(x, y) > 0) {
        score -= 15.0;  // More severe penalty
    }

    // Late-game scaling
    // if (turns > 50) {
    //     score *= 1.5;
    // }

    return score;
}

double Game::score_harvester(const Entity& e) {
    double score = 0.0;
    auto [dx, dy] = dirs[dir_idx.at(e.dir)];
    int px = e.x + dx, py = e.y + dy;
    
    if (!isValid(px, py) || !grid[px][py].is_protein()) 
        return -10000.0;
    
    if (harvested_by_me[px][py]) 
        return 0.0;  // Zero value for duplicates
    
    // Protein scarcity boost
    char ptype = grid[px][py].type[0];
    double need_factor = get_protein_need(ptype);
    score = 50.0 * need_factor;  // Increased from 25
    
    // Late-game depreciation
    if (turns > 75) score *= 0.7;
    
    return score;
}

double Game::score_sporer(const Entity& e) {
    double score = 0.0;
    auto [dx, dy] = dirs[dir_idx.at(e.dir)];
    
    // Root creation potential (70% weight), have enough proteins to grow sporer AND root
    bool can_afford_root = (me.p.a >= 1 && me.p.b >= 2 && me.p.c >= 1 && me.p.d >= 2);
    score = can_afford_root ? 40.0 : 10.0;  // Drastic increase
    
    // Line analysis (30% weight)
    int proteins = 0;
    for (int step = 1; ; step++) {
        int nx = e.x + dx*step, ny = e.y + dy*step;
        if (!isValid(nx, ny) || !grid[nx][ny].can_move_over()) break;
        if (grid[nx][ny].is_protein()) proteins++;
    }
    score += proteins * 15.0;  // Massive protein bonus

    // should add line of sight length bonus
    
    return score;
}

double Game::score_basic(const Entity& e) {
    double score = 2.0;  // Base value
    int x = e.x, y = e.y;
    
    // Protein collection (60% weight)
    if (grid[x][y].is_protein()) {
        char ptype = grid[x][y].type[0];
        double need_factor = get_protein_need(ptype);
        if (turns > 80 || me.p.a == 1) { // only at last or if needed
            score += 60.0 * need_factor;  // 3x boost for 3 proteins
        }
    }
    
    // Strategic positioning (40% weight)
    int d_to_enemy = distance[x][y];
    score += 15.0 / (1 + d_to_enemy);  // Encourage forward expansion
    
    return score;
}

double Game::score_root(const Entity& e) {
    double score = 100.0;  // Base value for new organism
    int x = e.x, y = e.y;
    
    // Protein density (40% weight)
    int nearby_proteins = 0;
    int found_adjecent = 0; // adjecent or onto
    for (int dx = -3; dx <= 3; dx++) {  // Larger radius
        for (int dy = -3; dy <= 3; dy++) {
            int nx = x + dx, ny = y + dy;
            if (isValid(nx, ny) && grid[nx][ny].is_protein()) {
                if (abs(nx - x) + abs(ny - y) <= 1) {
                    // found protein at distance <=1 (adjecent or onto root) from this root, should give penalty
                    found_adjecent++;
                }
                nearby_proteins++;
            }
        }
    }
    score += nearby_proteins * 20.0;  // Increased from 6
    score -= found_adjecent * 50;
    
    // Safety evaluation (30% weight)
    int d_to_enemy = distance[x][y];
    if (d_to_enemy <= 1) score -= 50.0;   // Extreme danger
    else if (d_to_enemy <= 2) score -= 20.0;  // High danger
    else if (d_to_enemy <= 3) score -= 5.0;   // Moderate danger

    if (controlled_by_enemy_tentacle(x, y) > 0) score -= 1000.0;
    
    // Expansion potential (30% weight)
    int expansion = 0;
    for (auto [dx, dy] : dirs) {
        int nx = x + dx, ny = y + dy;
        if (isValid(nx, ny) && grid[nx][ny].can_move_over()) {
            expansion++;
        }
    }
    score += expansion * 10.0;  // Increased from 1
    
    return score;
}


// Helper function for protein needs
double Game::get_protein_need(char protein_type) const {
    switch(protein_type) {
        case 'A': return 1.0 / (1.0 + me.p.a);
        case 'B': return 1.0 / (1.0 + me.p.b);
        case 'C': return 1.0 / (1.0 + me.p.c);
        case 'D': return 1.0 / (1.0 + me.p.d);
        default: return 0.0;
    }
}


void print_move(Entity &move) {
    if (move.type == "" || move.type == "WAIT") {
        cout << "WAIT" << endl;
        return;
    }
    string action = move.type == ROOT ? "SPORE" : "GROW";
    cout << action << " " << move.par_id << " " << move.x << " " << move.y;
    if (move.type != ROOT) {
        cout << " " << move.type;
    }
    if (move.type != BASIC && move.type != ROOT) {
        if (dir_idx.find(move.dir) == dir_idx.end()) {
            cerr << "no dir found!" << endl;
            move.dir = "E";
        }
        cout << " " << move.dir;
    }
    cout << endl;
}


int main() {
    cin >> W >> H;
    // cerr << "W, H : ";
    // cerr << W << " " << H << endl;

    Game game;

    while (1) {
        game.turns++;
        game.me.clear();
        game.he.clear();
        for (int x = 0; x < W; x++) {
            for (int y = 0; y < H; y++) {
                game.grid[x][y].reset();
                game.grid[x][y].x = x;
                game.grid[x][y].y = y;
            }
        }
        vector<Entity> my, his, proteins, others;

        int entity_count;
        cin >> entity_count;
        // cerr << "entity_count : ";
        // cerr << entity_count << endl;
        for (int i = 0; i < entity_count; i++) {
            Entity entity;
            entity.read();
            if (entity.is_mine()) {
                my.emplace_back(entity);
            } else if (entity.is_enemy()) {
                his.emplace_back(entity);
            } 
            // else if (entity.is_protein()) {
            //     proteins.emplace_back(entity);
            // } else {
            //     others.emplace_back(entity);
            // }
            game.grid[entity.x][entity.y] = entity;
        }

        // cerr << game.grid[16][8].type << endl;
        
        game.me.setup_organisms(my);
        game.he.setup_organisms(his);

        game.compute_harvested();
        game.set_dis_from_enemy();

        game.me.input_protein();
        game.he.input_protein();

        int required_actions_count; // your number of organisms, output an action for each one in any order
        cin >> required_actions_count;
        // cerr << "required_actions_count : ";
        // cerr << required_actions_count << endl;

        vector<Entity> moves = game.get_best_moves_list();
        for (auto &move : moves) {
            print_move(move);
        }
    }
}
















