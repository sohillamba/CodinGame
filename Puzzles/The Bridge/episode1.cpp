#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <assert.h>

using namespace std;

const int MAX_TURNS = 50;

const int SPEED_MOVE = 0;
const int SLOW_MOVE = 1;
const int WAIT_MOVE = 2;
const int JUMP_MOVE = 3;
const int UP_MOVE = 4;
const int DOWN_MOVE = 5;

const string MOVE_NAME[6] = {"SPEED", "SLOW", "WAIT", "JUMP", "UP", "DOWN"};


class State {
public:
    int data;       // 6 bits : speed
                    // 9 bits : 1 x coords (same for all active bikes)
                    // 4 bits : active (1) or inactive (0)
                    // 6 bits : turn count
                    // 3 bits : parent to this move

    State() : data(0) {}
    State(int d) : data(d) {}

    int getSpeed() const {
        return data & 0b111111;
    }
    void setSpeed(int val) {
        data = (data & ~(0b111111)) | (val);
    }

    int getX() const {
        return (data >> 6) & 0b111111111;
    }
    void setX(int val) {
        data = (data & ~(0b111111111 << 6)) | (val << 6);
    }

    bool isActive(int i) const { // i is treated as y coorinate
        return (data >> (15 + i)) & 0b1;
    }
    void setIsActive(int i, bool val) {
        data = (data & ~(0b1 << (15 + i))) | (val << (15 + i));
    }

    int getTurnCount() const {
        return (data >> 19) & 0b111111;
    }
    void setTurnCount(int val) {
        data = (data & ~(0b111111 << 19)) | (val << 19);
    }

    int getMove() const {
        return (data >> 25) & 0b111;
    }
    void setMove(int val) {
        data = (data & ~(0b111 << 25)) | (val << 25);
    }

    int encode() const {
        return data ^ (getTurnCount() << 19) ^ (getMove() << 25);
    }

    // void print() const {
    //     cerr << "State: ";
    //     cerr << "Speed: " << getSpeed();
    //     cerr << ", X: " << getX();
    //     cerr << ", Actives: " << isActive(0) << " " << isActive(1) << " " << isActive(2) << " " << isActive(3);
    //     cerr << ", Turn: " << getTurnCount();
    //     cerr << ", Move: " << MOVE_NAME[getMove()];
    //     cerr << endl;
    // }
};

struct maxHeapComp {
    bool operator () (const State &s1, const State &s2) {
        return s1.getX() < s2.getX();
    }
};


class Game {
public:
    int m, v, l;
    string road[4];
    vector<int> holeCount[4];

    State startState;
    vector<int> commands;

    Game() = default;

    void init() {
        startState.setTurnCount(0);
        cin >> m; cin.ignore();
        cin >> v; cin.ignore();
        for (int i = 0; i < 4; i++) {
            cin >> road[i]; cin.ignore();
            startState.setIsActive(i, false);
        }

        // extending roads by 10 non hole blocks, just to safegaurd the approach (extra moves are needed for test case 03)
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 10; j++) {
                road[i] += '.';
            }
        }

        l = road[0].size();
        for (int i = 0; i < 4; i++) {
            holeCount[i] = vector<int>(l, 0);
            holeCount[i][0] = road[i][0] == '0' ? 1 : 0;
            for (int j = 1; j < l; j++) {
                holeCount[i][j] = holeCount[i][j-1] + (road[i][j] == '0');
            }
        }

        // cerr << "Road length (l): " << l << endl;

        // cerr << "Hole count arr: " << endl;
        // for (int i = 0; i < 4; i++) {
        //     for (int j = 0; j < l; j++) {
        //         cerr << holeCount[i][j] << " ";
        //     }
        //     cerr << endl;
        // }
        // cerr << "-----" << endl;

        // for (int i = 0; i < l; i++) {
        //     cerr << i << " ";
        // }
        // cerr << endl;

        // cerr << "Roads: " << endl;
        // for (int i = 0; i < 4; i++) {
        //     for (int j = 0; j < l; j++) {
        //         cerr << road[i][j] << " ";
        //         if (j >= 10) cerr << " ";
        //     }
        //     cerr << endl;
        // }
        // cerr << "-----" << endl;
    }

    void read(const bool isFirst) {
        int speed;
        cin >> speed; cin.ignore();
        if (isFirst) {
            startState.setSpeed(speed);
        }
        for (int i = 0; i < m; i++) {
            int x, y, isActive;
            cin >> x >> y >> isActive; cin.ignore();
            if (isFirst) {
                startState.setX(x);
                startState.setIsActive(y, isActive == 1);
            }
        }
    }

    bool hasHole(int roadId, int i, int j) const {
        if (i >= l) return false;
        if (j >= l) j = l - 1;
        return holeCount[roadId][j] != (i == 0 ? 0 : holeCount[roadId][i-1]);
    }

    // move bike by running it, also update activity of bikes and return if good state or not
    bool moveBike(State &state, bool shouldUpdateX = true) {
        int active = 0;
        for (int i = 0; i < 4; i++) {
            if (state.isActive(i)) {
                if (hasHole(i, state.getX() + 1, state.getX() + state.getSpeed())) {
                    state.setIsActive(i, false);
                } else {
                    active++;
                }
            }
        }
        if (shouldUpdateX) {
            state.setX(state.getX() + state.getSpeed());
        }
        return active >= v;
    }

    bool speedMove(State &state) {
        state.setSpeed(state.getSpeed() + 1);
        state.setMove(SPEED_MOVE);
        return moveBike(state);
    }

    bool slowMove(State &state) {
        if (state.getSpeed() == 0) return false;
        state.setSpeed(state.getSpeed() - 1);
        state.setMove(SLOW_MOVE);
        return moveBike(state);
    }

    bool waitMove(State &state) {
        if (state.getSpeed() == 0) return false;
        state.setMove(WAIT_MOVE);
        return moveBike(state);
    }

    bool jumpMove(State &state) { // landing block can not be a hole
        if (state.getSpeed() == 0) return false;
        state.setX(state.getX() + state.getSpeed());
        state.setMove(JUMP_MOVE);
        int active = 0;
        for (int i = 0; i < 4; i++) {
            if (state.isActive(i)) {
                if (hasHole(i, state.getX(), state.getX())) {
                    state.setIsActive(i, false);
                } else {
                    active++;
                }
            }
        }
        return active >= v;
    }

    bool upMove(State &state) { // y = 0 is the highest lane
        if (state.getSpeed() == 0) return false;
        if (state.isActive(0)) return false;

        state.setSpeed(state.getSpeed() - 1);
        if (moveBike(state, false) == false) return false;
        state.setSpeed(state.getSpeed() + 1);

        for (int i = 1; i < 4; i++) {
            if (state.isActive(i)) {
                state.setIsActive(i - 1, true);
                state.setIsActive(i, false);
            }
        }
        state.setMove(UP_MOVE);
        return moveBike(state);
    }

    bool downMove(State &state) {
        if (state.getSpeed() == 0) return false;
        if (state.isActive(3)) return false;

        state.setSpeed(state.getSpeed() - 1);
        if (moveBike(state, false) == false) return false;
        state.setSpeed(state.getSpeed() + 1);

        for (int i = 2; i >= 0; i--) {
            if (state.isActive(i)) {
                state.setIsActive(i + 1, true);
                state.setIsActive(i, false);
            }
        }
        state.setMove(DOWN_MOVE);
        return moveBike(state);
    }

    void solve() {
        unordered_map<int, int> vis, parent;
        priority_queue<State, vector<State>, maxHeapComp> q;
        
        q.push(startState);
        vis[startState.encode()]++;

        State next, last;
        bool found = false;

        int mx = 0;

        while (!q.empty()) {
            State curState = q.top(); q.pop();
            int curData = curState.data;

            // if (parent.find(curData) != parent.end()) {
            //     cerr << "Parent: ----------------------------------------------" << endl;
            //     State par = State(parent[curData]);
            //     par.print();
            //     cerr << "Cur : --------------------------------" << endl;
            //     curState.print();
            //     cerr << "------------------------------------------------------" << endl;
            // }

            mx = max(mx, curState.getX());

            if (curState.getX() >= l) {
                // cerr << "Found with x: " << curState.getX() << " and speed: " << curState.getSpeed() << endl;
                last = curState;
                found = true;
                break;
            }

            curState.setTurnCount(curState.getTurnCount() + 1);
            if (curState.getTurnCount() >= MAX_TURNS) {
                continue;
            }

            auto pushToPQ = [&](const State &state) -> void {
                if (vis.find(state.encode()) == vis.end()) {
                    vis[state.encode()]++;
                    parent[state.data] = curData;
                    q.push(state);
                }
            };

            // speed
            next = curState;
            if (speedMove(next)) {
                pushToPQ(next);
            }

            // slow
            next = curState;
            if (slowMove(next)) {
                pushToPQ(next);
            }

            // wait
            // next = curState;
            // if (waitMove(next)) {
            //     pushToPQ(next);
            // }

            // jump
            next = curState;
            if (jumpMove(next)) {
                pushToPQ(next);
            }

            // up
            next = curState;
            if (upMove(next)) {
                pushToPQ(next);
            }

            // down
            next = curState;
            if (downMove(next)) {
                pushToPQ(next);
            }
        }

        // cerr << "Max x reached in bfs: " << mx << endl;

        if (found) {
            while (last.data != startState.data) {
                commands.push_back(last.getMove());
                last = parent[last.data];
            }
            reverse(commands.begin(), commands.end());
            // cerr << "Commands size: " << commands.size() << endl;
            // for (int c : commands) {
            //     cerr << MOVE_NAME[c] << ", ";
            // }
            // cerr << endl;
        } else {
            assert(false);
        }
    }

    string getCommand(int turn) const {
        turn -= 1;
        assert(turn < (int)commands.size()); // can make WAIT move in this case
        return MOVE_NAME[commands[turn]];
    }
};




int main() {
    Game game;
    game.init();

    for (int turn = 1; ; turn++) {
        // break;
        game.read(turn == 1);
        if (turn == 1) {
            game.solve();
        }
        cout << game.getCommand(turn) << endl;
    }
}





