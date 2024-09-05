/*
  working logic for collecting maximum goals with shortest path (PUSH turn)
  and somewhat working logic for PUSH turn (Dont know after some time it always pushes 1st col vertically up)
*/

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <queue>
#include <cstring>

using namespace std;


const int N = 7;
const int dir_row[4] = {-1, 0, 1, 0};
const int dir_col[4] = {0, 1, 0, -1};
const string dir_name[4] = {"UP", "RIGHT", "DOWN", "LEFT"};
const int PUSH_TURN = 0, MOVE_TURN = 1;
// const int me.id = 0, HIS_ID = 1;
const int MAX_MOVES = 20;


bool is_horizontal (int dir) {
    return dir & 1;
}
bool is_vertival (int dir) {
    return !is_horizontal(dir);
}
void read_grid(vector<vector<string>> &grid) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            cin >> grid[i][j]; cin.ignore();
        }
    }
}
int get_opposite_dir (int dir) {
    return dir ^ 2;
}
int is_inside (int row, int col) {
    return row >= 0 && row < N && col >= 0 && col < N;
}
bool can_move (vector<vector<string>> &grid, int row, int col, int dir) {
    int r2 = row + dir_row[dir];
    int c2 = col + dir_col[dir];
    return grid[row][col][dir] == '1' && is_inside(r2, c2) && grid[r2][c2][get_opposite_dir(dir)] == '1';
}
int get_distance (pair<int, int> a, pair<int, int> b) {
    return abs(a.first - b.first) + abs(a.second - b.second);
}

struct Player {
    int id;
    int cards_count;
    int row, col;
    string tile;
    vector<pair<int, int>> goals;

    Player (int _id) {
        id = _id;
    }
    void read_input () {
        cin >> cards_count >> col >> row >> tile; cin.ignore();
    }
    bool can_move (vector<vector<string>> &grid, int dir) {
        return :: can_move(grid, row, col, dir);
    }
    void go (int dir) {
        row += dir_row[dir];
        col += dir_col[dir];
    }
};
struct Item {
    string name;
    int row, col;
    int player;

    void read_input () {
        cin >> name >> col >> row >> player; cin.ignore();
        // string temp = to_string(item_count);
        // if (temp.size() == 1) temp = "0" + temp;
        // grid[row][col] += temp;
        // item_count++;
    }
};
struct Push {
    int id;
    bool is_right_down;
    bool is_row;
    Push (int _id, bool _is_right_down, bool _is_row) {
        id = _id;
        is_right_down = _is_right_down;
        is_row = _is_row;
    }
};

// struct State {
//     string grid[N][N];
//     Player me, he;

//     void read_items () {
//         int num_items; // the total number of items available on board and on player tiles
//         cin >> num_items; cin.ignore();

//         vector<Item> items(num_items);
//         for (Item &item : items) {
//             item.read_input();
//         }
//     }
// };

// struct Game {
//     string grid[N][N];
//     player me, he;
//     vector<Item> items;


// };


// return all tiles which are accessible from the given tile
vector<pair<int, int>> get_all_accessible_tiles (vector<vector<string>> &grid, Player &me) {
    queue<pair<int, int>> q;
    int vis[N][N];
    
    memset(vis, 0, sizeof vis);
    q.push({me.row, me.col});
    vis[me.row][me.col] = 1;

    vector<pair<int, int>> tiles;

    int depth = 0;
    while (!q.empty() && depth < MAX_MOVES) {
        int sz = q.size();
        while (sz--) {
            pair<int, int> cur = q.front(); q.pop();
            tiles.push_back(cur);
            for (int dir = 0; dir < 4; dir++) {
                int r2 = cur.first + dir_row[dir];
                int c2 = cur.second + dir_col[dir];
                if (is_inside(r2, c2) && !vis[r2][c2] && can_move(grid, cur.first, cur.second, dir)) {
                    vis[r2][c2] = 1;
                    q.push(make_pair(r2, c2));
                }
            }
        }
        depth++;
    }

    return tiles;
}


// returns the shortest path between given 2 tiles 
vector<pair<int, int>> get_path_from_to (vector<vector<string>> &grid, pair<int, int> from, pair<int, int> to) {
    queue<pair<int, int>> q;
    pair<int, int> parent[N][N];
    int vis[N][N];

    memset(vis, 0, sizeof vis);
    q.push(from);
    vis[from.first][from.second] = 1;
    parent[from.first][from.second] = {-1, -1};

    int depth = 0;
    while (!q.empty() && depth < MAX_MOVES) {
        int sz = q.size();
        while (sz--) {
            pair<int, int> cur = q.front(); q.pop();
            if (cur == to) {
                vector<pair<int, int>> path;
                while (cur != from) {
                    path.push_back(cur);
                    cur = parent[cur.first][cur.second];
                }
                path.push_back(from);
                reverse(path.begin(), path.end());
                return path;
            }
            for (int dir = 0; dir < 4; dir++) {
                int r2 = cur.first + dir_row[dir];
                int c2 = cur.second + dir_col[dir];
                if (is_inside(r2, c2) && !vis[r2][c2] && can_move(grid, cur.first, cur.second, dir)) {
                    parent[r2][c2] = cur;
                    vis[r2][c2] = 1;
                    q.push(make_pair(r2, c2));
                }
            }
        }
        depth++;
    }
    return {};
}

// return moves for a path
vector<int> get_moves_from_path (vector<pair<int, int>> &path) {
    vector<int> moves;
    for (int i = 1; i < (int)path.size(); i++) {
        for (int dir = 0; dir < 4; dir++) {
            int r2 = path[i-1].first + dir_row[dir], c2 = path[i-1].second + dir_col[dir];
            if (r2 == path[i].first && c2 == path[i].second) {
                moves.push_back(dir);
                break;
            }
        }
    }
    return moves;
}


// return moves of the path which end upon a goal or gets nearest to a goal
vector<int> get_moves(vector<vector<string>> &grid, Player &me) {
    if (me.goals.empty()) return {};

    queue<pair<int, int>> q;
    pair<int, int> parent[N][N];
    int vis[N][N], parent_to_child_dir[N][N];

    memset(vis, 0, sizeof vis);
    memset(parent_to_child_dir, -1, sizeof parent_to_child_dir);

    q.push({me.row, me.col});
    vis[me.row][me.col] = 1;
    parent[me.row][me.col] = {-1, -1};

    int best_dis = 1e9;
    for (auto &goal : me.goals) {
        best_dis = min(best_dis, get_distance(goal, {me.row, me.col}));
    }
    pair<int, int> best_pos = {-1, -1};

    int depth = 0;
    while (!q.empty() && depth < MAX_MOVES) {
        int sz = q.size();
        while (sz--) {
            pair<int, int> cur = q.front(); q.pop();
            for (int dir = 0; dir < 4; dir++) {
                int r2 = cur.first + dir_row[dir];
                int c2 = cur.second + dir_col[dir];
                if (is_inside(r2, c2) && !vis[r2][c2] && can_move(grid, cur.first, cur.second, dir)) {
                    parent[r2][c2] = cur;
                    parent_to_child_dir[r2][c2] = dir;
                    vis[r2][c2] = 1;
                    q.push(make_pair(r2, c2));

                    int dis = 1e9;
                    for (auto &goal : me.goals) {
                        dis = min(dis, get_distance(goal, {r2, c2}));
                    }
                    if (dis < best_dis) {
                        best_dis = dis;
                        best_pos = make_pair(r2, c2);
                    }
                }
            }
        }
        depth++;
    }

    vector<int> moves;
    if (best_pos != make_pair(-1, -1)) {
        pair<int, int> cur = best_pos;
        while (parent[cur.first][cur.second] != make_pair(-1, -1)) {
            moves.push_back(parent_to_child_dir[cur.first][cur.second]);
            cur = parent[cur.first][cur.second];
        }
        reverse(moves.begin(), moves.end());
    }
    return moves;
}


// return moves of the path which is shortest yet collecting maximum goals
// TODO: after moving to last goal, move to the best final tile
vector<int> get_best_moves_path (vector<vector<string>> &grid, Player &me) {
    vector<pair<int, int>> tiles = get_all_accessible_tiles(grid, me);

    vector<int> goal_tiles;
    // TODO: make more effecient (dont think it will improve time)
    for (int i = 0; i < tiles.size(); i++) {
        for (auto &goal : me.goals) {
            if (goal.first == tiles[i].first && goal.second == tiles[i].second) {
                goal_tiles.push_back(i);
            }
        }
    }

    if (goal_tiles.size() < 2) {
        // if 0 goal tile -> move to best tile
        // if 1 goal tile -> move to goal tile and then move to best tile (TODO)
        return get_moves(grid, me);
    }

    vector<pair<int, int>> best_path;
    int max_goals_visited = 0;
    do {
        pair<int, int> cur = make_pair(me.row, me.col);
        vector<pair<int, int>> cur_path;
        cur_path.push_back(cur);
        int goals_visited = 0;
        for (int &i : goal_tiles) {
            vector<pair<int, int>> path_to_this_goal = get_path_from_to(grid, cur, tiles[i]);
            if (path_to_this_goal.empty() || cur_path.size() + path_to_this_goal.size() - 1 > MAX_MOVES) {
                break;
            }
            cur_path.insert(cur_path.end(), path_to_this_goal.begin() + 1, path_to_this_goal.end());
            cur = tiles[i];
            goals_visited++;
        }
        if (goals_visited > max_goals_visited || 
            (goals_visited == max_goals_visited && cur_path.size() < best_path.size())) {
            best_path = cur_path;
            max_goals_visited = goals_visited;
        }
    } while (next_permutation(goal_tiles.begin(), goal_tiles.end()));

    return get_moves_from_path(best_path);
}



void do_push(Push push, vector<vector<string>> &grid, Player &p) {
    if (push.is_row) {
        // push row, dir => left, right (0, 1)
        int row = push.id;
        if (push.is_right_down == 0) {
            // <----- this rotation
            p.row = (p.row + N - 1) % N;
            string temp = grid[row][0];
            for (int col = 0; col < N - 1; col++) {
                grid[row][col] = grid[row][col+1];
            }
            grid[row][N-1] = p.tile;
            p.tile = temp;
        } else {
            p.row = (p.row + 1) % N;
            string temp = grid[row][N-1];
            for (int col = N - 1; col > 0; col--) {
                grid[row][col] = grid[row][col-1];
            }
            grid[row][0] = p.tile;
            p.tile = temp;
        }
    } else {
        // push column, dir => up, down (0, 1)
        int col = push.id;
        if (push.is_right_down == 0) {
            p.col = (p.col - 1 + N) % N;
            string temp = grid[0][col];
            for (int row = 0; row < N - 1; row++) {
                grid[row][col] = grid[row+1][col];
            }
            grid[N-1][col] = p.tile;
            p.tile = temp;
        } else {
            p.col = (p.col + 1) % N;
            string temp = grid[N-1][col];
            for (int row = N - 1; row > 0; row--) {
                grid[row][col] = grid[row-1][col];
            }
            grid[0][col] = p.tile;
            p.tile = temp;
        }
    }
}


int eval (vector<vector<string>> &grid, vector<Item> &items, Player &me, Player &he) {
    /*
        10000 for vistory
        100 for each completed quests
        20 for all accessibles items
        50 for all quests item that are on the border (not for now)
    */
    if (me.cards_count == 0) return 10000;
    vector<pair<int, int>> tiles = get_all_accessible_tiles(grid, me);
    int item_accessible = 0, goal_accessible = 0;
    for (auto &tile : tiles) {
        for (auto &item : items) {
            if (item.row == tile.first && item.col == tile.second && item.player == me.id) {
                item_accessible++;
            }
        }
        for (auto &goal : me.goals) {
            if (goal.first == tile.first && goal.second == tile.second) {
                goal_accessible++;
            }
        }
    }

    int score = 0;
    score += 20 * (item_accessible - goal_accessible);
    score += 100 * (goal_accessible);

    // // not for now
    // for (auto &goal : goals) {
    //     if (goal.first == 0 || goal.first == N - 1 || goal.second == 0 || goal.second == N - 1) {
    //         score += 50;
    //     }
    // }

    return score;
}


Push get_best_push (vector<vector<string>> &_grid, vector<Item> &items, Player &me, Player &he) {
    vector<vector<string>> grid = _grid;

    int best_score = -1e9;
    Push best(-1, 0, 0);

    for (int i = 0; i < N; i++) {
        for (int is_right_down = 0; is_right_down <= 1; is_right_down++) {
            for (int is_row = 0; is_row <= 1; is_row++) {
                do_push(Push(i, is_right_down, is_row), grid, me);
                int score = eval(grid, items, me, he);
                if (score > best_score) {
                    best_score = score;
                    best = Push(i, is_right_down, is_row);
                }
                do_push(Push(i, !is_right_down, is_row), grid, me);
            }
        }
    }

    return best;
}


int main()
{
    vector<vector<string>> grid(N, vector<string> (N));
    Player me(0), he(1);
    int push_try = 0;

    // game loop
    while (1) {
        int turn_type;
        cin >> turn_type; cin.ignore();
        
        read_grid(grid);

        me.read_input();
        he.read_input();

        int num_items; // the total number of items available on board and on player tiles
        cin >> num_items; cin.ignore();
        vector<Item> items(num_items);
        for (Item &item : items) {
            item.read_input();
        }

        int num_quests; // the total number of revealed quests for both players
        cin >> num_quests; cin.ignore();
        
        me.goals.clear();
        he.goals.clear();
        for (int i = 0; i < num_quests; i++) {
            string item_name;
            int player_id;
            cin >> item_name >> player_id; cin.ignore();

            for (Item &item : items) {
                if (item.name == item_name && item.player == player_id) {
                    if (player_id == me.id) {
                        me.goals.push_back(make_pair(item.row, item.col));
                    } else {
                        he.goals.push_back(make_pair(item.row, item.col));
                    }
                }
            }
        }


        if (turn_type == MOVE_TURN) {
            // vector<int> moves = get_moves(grid, me);
            vector<int> moves = get_best_moves_path(grid, me);
            if (!moves.empty()) {
                cout << "MOVE";
                for (int i = 0; i < min(20, (int)moves.size()); i++) {
                    cout << " " << dir_name[moves[i]];
                }
                cout << endl;
            } else {
                cout << "PASS" << endl;
            }
        } else {
            Push push = get_best_push(grid, items, me, he);
            // push.id = -1;
            if (push.id != -1) {
                push_try++;
                string dir;
                if (push.is_row) {
                    dir = push.is_right_down ? "RIGHT" : "LEFT";
                } else {
                    dir = push.is_right_down ? "DOWN" : "UP";
                }
                cout << "PUSH " << push.id << " " << dir << endl;
                cerr << push_try << endl;
            } else {
                int dir = rand() % 4;
                int to_push = is_horizontal(dir) ? me.row : me.col;
                cout << "PUSH " << to_push << " " << dir_name[dir] << endl;
                cerr << push_try << endl;
            }
        }
    }

    return 0;
}










