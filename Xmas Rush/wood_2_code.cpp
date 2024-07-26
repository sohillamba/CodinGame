#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;


const int N = 7;
const int dir_row[4] = {-1, 0, 1, 0};
const int dir_col[4] = {0, 1, 0, -1};
const string dir_name[4] = {"UP", "RIGHT", "DOWN", "LEFT"};
const int PUSH_TURN = 0, MOVE_TURN = 1;


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
        for (int i = 0; i < num_items; i++) {
            string item_name;
            int item_x;
            int item_y;
            int item_player_id;
            cin >> item_name >> item_x >> item_y >> item_player_id; cin.ignore();
        }
        int num_quests; // the total number of revealed quests for both players
        cin >> num_quests; cin.ignore();
        for (int i = 0; i < num_quests; i++) {
            string quest_item_name;
            int quest_player_id;
            cin >> quest_item_name >> quest_player_id; cin.ignore();
        }



        if (turn_type == MOVE_TURN) {
            vector<int> moves, order = {0, 1, 2, 3};
            for (int i = 0; i < 20; i++) {
                random_shuffle(order.begin(), order.end());
                for (int dir : order) {
                    if (me.can_move(dir)) {
                        moves.push_back(dir);
                        me.row += dir_row[dir];
                        me.col += dir_col[dir];
                        break;
                    }
                }
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
