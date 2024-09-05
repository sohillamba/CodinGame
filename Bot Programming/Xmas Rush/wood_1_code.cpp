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
const int MY_ID = 0, HIS_ID = 1;


string grid[N][N];

bool is_horizontal (int dir) {
    return dir & 1;
}

bool is_vertival (int dir) {
    return !is_horizontal(dir);
}

void read_grid() {
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

bool can_move (int row, int col, int dir) {
    int r2 = row + dir_row[dir];
    int c2 = col + dir_col[dir];
    return grid[row][col][dir] == '1' && is_inside(r2, c2) && grid[r2][c2][get_opposite_dir(dir)] == '1';
}

int get_distance (pair<int, int> a, pair<int, int> b) {
    return abs(a.first - b.first) + abs(a.second - b.second);
}

struct Player {
    int cards_count;
    int row, col;
    string tile;

    void read_input () {
        cin >> cards_count >> col >> row >> tile; cin.ignore();
    }

    bool can_move (int dir) {
        return :: can_move(row, col, dir);
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
    }
};


int main()
{

    // game loop
    while (1) {
        int turn_type;
        cin >> turn_type; cin.ignore();
        
        read_grid();

        Player me, he;
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
        
        pair<int, int> goal = {-1, -1};
        for (int i = 0; i < num_quests; i++) {
            string item_name;
            int player_id;
            cin >> item_name >> player_id; cin.ignore();

            if (player_id == MY_ID) {
                for (Item &item : items) {
                    if (item.name == item_name && item.player == player_id) {
                        goal = make_pair(item.row, item.col);
                        break;
                    }
                }
            }
        }


        if (turn_type == MOVE_TURN) {
            if (goal == make_pair(-1, -1)) {
                cout << "PASS" << endl;
                continue;
            }

            queue<pair<int, int>> q;
            q.push({me.row, me.col});

            int vis[N][N];
            memset(vis, 0, sizeof vis);
            vis[me.row][me.col] = 1;

            pair<int, int> parent[N][N];
            parent[me.row][me.col] = {-1, -1};

            int parent_to_child_dir[N][N];
            memset(parent_to_child_dir, -1, sizeof parent_to_child_dir);

            int best_dis = get_distance(goal, make_pair(me.row, me.col));
            pair<int, int> best_pos = {-1, -1};

            int depth = 0;
            while (!q.empty() && depth < 20) {
                int sz = q.size();
                while (sz--) {
                    pair<int, int> cur = q.front(); q.pop();
                    for (int dir = 0; dir < 4; dir++) {
                        int r2 = cur.first + dir_row[dir];
                        int c2 = cur.second + dir_col[dir];
                        if (is_inside(r2, c2) && !vis[r2][c2] && can_move(cur.first, cur.second, dir)) {
                            parent[r2][c2] = cur;
                            parent_to_child_dir[r2][c2] = dir;
                            vis[r2][c2] = 1;
                            q.push(make_pair(r2, c2));

                            int dis = get_distance(make_pair(r2, c2), goal);
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

            if (!moves.empty()) {
                cout << "MOVE";
                for (int move : moves) {
                    cout << " " << dir_name[move];
                }
                cout << endl;
            } else {
                cout << "PASS" << endl;
            }
        } else {
            int dir = rand() % 4;
            int to_push = is_horizontal(dir) ? me.row : me.col;
            cout << "PUSH " << to_push << " " << dir_name[dir] << endl;
        }
    }
}
