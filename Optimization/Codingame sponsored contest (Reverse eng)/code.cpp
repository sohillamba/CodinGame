// #pragma GCC optimize("-O3","-ffast-math")
// #pragma GCC optimize("inline")
// #pragma GCC optimize("omit-frame-pointer")
// #pragma GCC optimize("unroll-loops")

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <queue>

using namespace std;


/*
    output is direction
    A: down
    B: wait
    C: left
    D: right
    E: up

    characters are wall('#') or space('_')
    char a; // left
    char b; // down
    char c; // right
    char d; // up

    m(width) is given first than n(height)

    k is always 5 (4 emenies, 1 player(5th))

    1 based coordinate system
    top left: {1, 1}

    game ends upon touching enemy               (not sure if true in all directions)

    score: 2*(total distinct cells visited)     (not sure)
*/

const int DOWN = 0;
const int WAIT = 1;
const int LEFT = 2;
const int RIGHT = 3;
const int UP = 4;

const int DIR[4] = {LEFT, RIGHT, DOWN, UP}; // just to iterate, order does not matter

const char ALPHA[5] = {'A', 'B', 'C', 'D', 'E'};


class Point {
public:
    int x, y;
    int prevDir;
    Point() : x(-1), y(-1), prevDir(WAIT) {}
    Point(const int& x_, const int& y_) : x(x_), y(y_), prevDir(WAIT) {}
    Point(const int& x_, const int& y_, const int &d) : x(x_), y(y_), prevDir(d) {}

    Point move(int dir) {
        if (dir == DOWN) return Point(x + 1, y, dir);
        if (dir == LEFT) return Point(x, y - 1, dir);
        if (dir == RIGHT) return Point(x, y + 1, dir);
        if (dir == UP) return Point(x - 1, y, dir);
        return Point(x, y, WAIT);
    }
};

class Game {
public:
    int n, m, k;
    char left, down, right, up;
    Point player;
    vector<Point> enemies, histry;
    vector<vector<char>> grid;

    Game () {
        cin >> m >> n >> k;
        cerr << n << ", " << m << ", " << k << endl;

        enemies.resize(k-1);
        histry.clear();
        grid.resize(n + 1, vector<char> (m + 1, '?'));
    }

    bool inside(int x, int y) {
        return x > 0 && x <= n && y > 0 && y <= m;
    }
    bool inside(Point p) {
        return inside(p.x, p.y);
    }

    void setGridCell(Point p, char value) {
        // if (value == '_') value = ' ';
        if (inside(p) && grid[p.x][p.y] != ' ') grid[p.x][p.y] = value;
    }
    void setPrintGridCell(Point p, char value, vector<vector<char>> &g) {
        if (inside(p)) g[p.x][p.y] = value;
    }
    char getGridCell(Point p) {
        return inside(p) ? grid[p.x][p.y] : '?';
    }

    void print() {
        auto cur = grid;
        for (int i = 0; i < k - 1; i++) {
            setPrintGridCell(enemies[i], 'A' + i, cur);
        }
        setPrintGridCell(player, 'A' + k - 1, cur);

        for (int i = 1; i <= n; i++) {
            for (int j = 1; j <= m; j++) {
                cerr << cur[i][j];
            }
            cerr << endl;
        }
        cerr << endl;
    }

    void read() {
        cin >> left >> down >> right >> up;
        cerr << left << ", " << down << ", " << right << ", " << up << endl;
        for (int i = 0; i < k - 1; i++) {
            cin >> enemies[i].x >> enemies[i].y;
            cerr << enemies[i].x << " " << enemies[i].y << endl;
            if (getGridCell(enemies[i]) != ' ') {
                setGridCell(enemies[i], '_');
            } 
        }

        histry.push_back(player);
        cin >> player.x >> player.y;
        cerr << player.x << " " << player.y << endl;
        setGridCell(player, ' ');

        setGridCell(player.move(LEFT), left);
        setGridCell(player.move(DOWN), down);
        setGridCell(player.move(RIGHT), right);
        setGridCell(player.move(UP), up);
    }

    bool canMove(Point p) {
        return grid[p.x][p.y] == ' ' || grid[p.x][p.y] == '_';
    }
    bool hasEnemy(Point p) {
        for (auto &enemy : enemies) {
            if (enemy.x == p.x && enemy.y == p.y) return true;
        }
        return false;
    }

    vector<Point> getPossibleMoves(Point u) {
        vector<Point> possibleMoves;
        for (int dir : DIR) {
            Point v = u.move(dir);
            if (inside(v) && canMove(v) && !hasEnemy(v)) {
                possibleMoves.push_back(v);
            }
        }
        return possibleMoves;
    }

    Point getNearestUnknown() {
        queue<pair<Point, vector<Point>>> q;
        vector<vector<int>> vis(n + 2, vector<int> (m + 2, 0));
        q.push({player, {}});
        while (!q.empty()) {
            auto [u, path] = q.front();
            q.pop();
            
            if (vis[u.x][u.y]) continue;
            vis[u.x][u.y] = 1;

            if (grid[u.x][u.y] == '_') return path.empty() ? player : path[0];
            for (Point v : getPossibleMoves(u)) {
                path.push_back(v);
                q.push({v, path});
                path.pop_back();
            }
        }
        return player;
    }

    char getCommand() {
        // try to explore nearest unknown
        Point unknown = getNearestUnknown();
        if (unknown.x != player.x || unknown.y != player.y) {
            return ALPHA[unknown.prevDir];
        }

        // try possible safe movement
        vector<Point> possibleMoves = getPossibleMoves(player);
        if (!possibleMoves.empty()) {
            return ALPHA[possibleMoves[rand() % possibleMoves.size()].prevDir];
        }

        // try to hit enemy
        for (int dir : DIR) {
            Point p = player.move(dir);
            if (inside(p) && canMove(p) && hasEnemy(p)) {
                return ALPHA[dir];
            }
        }

        // wait
        return ALPHA[WAIT];
    }
};

int main() {
    ios_base::sync_with_stdio(false);cin.tie(NULL);cout.tie(NULL);
    srand(time(NULL));

    Game game;

    while (1) {
        game.read();
        game.print();

        cout << game.getCommand() << endl;
    }
}






