/*
    
    Simple algo: (just like privious file (silver_entry.cpp) with some extra move generation contitions) just some move generation logic with some conditions and then the first move of this list is played
      No simulation or minmax or anything is used. simple logic
      (just sort the moves and choose the first one)
      (move generation and sorting logic is written in move generation function (getAllActions())
    
    Inspired by: https://forum.codingame.com/t/spring-challenge-2021-feedbacks-strategies/190849/2
    user: (eulerscheZahl) https://forum.codingame.com/u/eulerscheZahl

    And: https://forum.codingame.com/t/spring-challenge-2021-feedbacks-strategies/190849/33
    user: (Kodle) https://forum.codingame.com/u/Kodle
*/

#pragma GCC optimize("-O3","-ffast-math")
#pragma GCC optimize("inline")
#pragma GCC optimize("omit-frame-pointer")
#pragma GCC optimize("unroll-loops")

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <cstring>
#include <sstream>
#include <set>
#include <unordered_map>

using namespace std;

const int N = 37;
const int ME = 1;
const int HE = 0;

const int GROW = 0;
const int SEED = 1;
const int COMPLETE = 2;
const int WAIT = 3;

// State related constants
const int DEPTH_BITS = 48;
const int ACTION_BITS = 48;
const int SCORE_BITS = 48;
const int NUTRIENTS_BITS = 48;
const int SUN_BITS = 56;
const int DAY_BITS = 56;

int RICHNESS[N];
int NEIGH[N][6];
vector<int> REACHABLE_CELLS[N][4];      // reachable cells from cell i up to delpth d
vector<int> SHADOW_CELLS[N][4][6];      // shadow[i][j][k] = shadow cells for a tree at index i and of size j and with day k

uint64_t SOIL_ZERO;                     // 37-bit mask where 1 represent that this cell has zero richness
uint64_t REACHABLE_CELLS_MASK[N][4];    // 37-bit mask (all index for cell i with depth j)
uint64_t SHADOW_CELLS_MASK[N][4][6];    // 37-bit mask (all index for cell i with tree size j and day k)

int otherPId (int pId) {
    return (pId == ME) ? HE : ME;
}

int seedingCost(int treeCount) {
    return treeCount;
}
int growCost(int treeSize, int treeCount) { // tree size and the count of trees of (array of 4 ([0-3] size tree count))
    if (treeSize == 0) {        // seed to small tree
        return 1 + treeCount;
    } else if (treeSize == 1) { // small to medium tree
        return 3 + treeCount;
    } else {                    // medium to large tree
        return 7 + treeCount;
    }
}
int completionCost() {
    return 4;
}


int getSunDirection (int day) {
    /*
    directions:
    * * * * * *
    * * * * * *
     * * 2 1 * *
    * * 3 # 0 *
     * * 4 5 * *
    * * * * * *
    */
    return day % 6;
}
int getShadowDirection (int day) {
    // return (day + 3) % 6;
    return (getSunDirection(day) + 3) % 6;
}


int getCompletionBonusScore (int index) {
    if (RICHNESS[index] == 1) {
        return 0;
    } else if (RICHNESS[index] == 2) {
        return 2;
    } else if (RICHNESS[index] == 3) {
        return 4;
    } else {
        return 0;
    }
}


int cntBits(uint64_t i) {
    // adopted from: https://stackoverflow.com/a/109025
    i = i - ((i >> 1) & 0x5555555555555555);  // add pairs of bits
    i = (i & 0x3333333333333333) + ((i >> 2) & 0x3333333333333333);  // quads
    i = (i + (i >> 4)) & 0x0F0F0F0F0F0F0F0F;  // groups of 8
    i = i + (i >> 8);  // sum groups of 16
    i = i + (i >> 16); // sum groups of 32
    i = i + (i >> 32); // sum groups of 64
    return i & 0x7F;   // return the sum of bits (limited to the relevant bits)
}



class Action {
public:
    uint16_t data;  // bit 0-2  = type
                    // bit 3-8  = action index
                    // bit 9-14 = target index (only in case of seeding (target index))

    // int type, index1, index2;

    Action () {
        // setType(4);
        // setIndex1(37);
        // setIndex2(37);
        data = 4 + (37 << 3) + (37 << 9);
    }

    Action (int t) {
        // setType(t);
        data = t;
    }

    Action (int t, int id) {
        // setType(t);
        // setIndex1(id);
        data = t + (id << 3);
    }

    Action (int t, int id1, int id2) {
        // setType(t);
        // setIndex1(id1);
        // setIndex2(id2);
        data = t + (id1 << 3) + (id2 << 9);
    }

    uint16_t getType() const {
        return (data) & 0b111;
    }
    uint16_t getIndex1() const {
        return (data >> 3) & 0b111111;
    }
    uint16_t getIndex2() const {
        return (data >> 9) & 0b111111;
    }

    void setType(int t) {
        data = (data & ~0b111) | t;
    }
    void setIndex1(int val) {
        data = (data & ~(0b111111 << 3)) | (val << 3);
    }
    void setIndex2(int val) {
        data = (data & ~(0b111111 << 9)) | (val << 9);
    }
};


struct State {
    uint64_t seed;          // | first 37 bits CELL has SEED        | --11 bits free-- |       16 bits Depth       |
    uint64_t sm;            // | first 37 bits CELL has size 1 TREE | --11 bits free-- |       16 bits Actions     |
    uint64_t md;            // | first 37 bits CELL has size 2 TREE | -----------------27 bits free----------------|
    uint64_t ta;            // | first 37 bits CELL has size 3 TREE | -----------------27 bits free----------------|
    uint64_t me[2];         // | first 37 bits CELL has OWN tree    | --11 bits free-- | 8 bits SCORE | 8 bits SUN |
    // uint64_t opp;        // | first 37 bits CELL has OPP tree    | --11 bits free-- | 8 bits SCORE | 8 bits SUN |
    uint64_t dormant;       // | first 37 bits CELL is DORMANT      | --11 bits free-- | 8 bits NUTRI | 8 bits DAY |

    // TODO: check if necessory (or optimise it, currently not optimised)
    bool playerWaiting[2];

    State () {
        seed = sm = md = ta = me[0] = me[1] = dormant = 0ULL;
        playerWaiting[0] = playerWaiting[1] = 0;
    }

    void clear() {
        seed = sm = md = ta = me[0] = me[1] = dormant = 0ULL;
        playerWaiting[0] = playerWaiting[1] = 0;
    }

    bool hasSeed(int cellIndex) const {
        return (seed >> cellIndex) & 1ULL;
    }
    void setSeed(int cellIndex, bool value) {
        if (value) {
            seed |= (1ULL << cellIndex);
        } else {
            seed &= ~(1ULL << cellIndex);
        }
    }

    bool hasTree(int cellIndex) const {
        return hasTreeSize1(cellIndex) | hasTreeSize2(cellIndex) | hasTreeSize3(cellIndex);
    }
    bool hasTreeSize1(int cellIndex) const {
        return (sm >> cellIndex) & 1ULL;
    }
    bool hasTreeSize2(int cellIndex) const {
        return (md >> cellIndex) & 1ULL;
    }
    bool hasTreeSize3(int cellIndex) const {
        return (ta >> cellIndex) & 1ULL;
    }

    void setTreeSize1(int cellIndex, bool value) {
        if (value) {
            sm |= (1ULL << cellIndex);
        } else {
            sm &= ~(1ULL << cellIndex);
        }
    }
    void setTreeSize2(int cellIndex, bool value) {
        if (value) {
            md |= (1ULL << cellIndex);
        } else {
            md &= ~(1ULL << cellIndex);
        }
    }
    void setTreeSize3(int cellIndex, bool value) {
        if (value) {
            ta |= (1ULL << cellIndex);
        } else {
            ta &= ~(1ULL << cellIndex);
        }
    }

    bool isPlayersTree(int cellIndex, int pId) const {
        return (me[pId] >> cellIndex) & 1ULL;
    }
    void setPlayersTree(int cellIndex, int pId, bool value) {
        if (value) {
            me[pId] |= (1ULL << cellIndex);
        } else {
            me[pId] &= ~(1ULL << cellIndex);
        }
    }

    bool isDormant(int cellIndex) const {
        return (dormant >> cellIndex) & 1ULL;
    }
    void setDormant(int cellIndex, bool value) {
        if (value) {
            dormant |= (1ULL << cellIndex);
        } else {
            dormant &= ~(1ULL << cellIndex);
        }
    }

    uint8_t getPlayersScore(int pId) const  {
        return (me[pId] >> SCORE_BITS) & 0xFF;
    }
    void setPlayersScore(int pId, uint8_t score) {
        me[pId] = (me[pId] & ~(0xFFULL << SCORE_BITS)) | (static_cast<uint64_t>(score) << SCORE_BITS);
    }

    // Get and set players SUN points
    uint8_t getPlayersSun(int pId) const {
        return (me[pId] >> SUN_BITS) & 0xFF;
    }
    void setPlayersSun(int pId, uint8_t sun) {
        me[pId] = (me[pId] & ~(0xFFULL << SUN_BITS)) | (static_cast<uint64_t>(sun) << SUN_BITS);
    }

    // Get and set nutrients value
    uint8_t getNutrients() const {
        return (dormant >> NUTRIENTS_BITS) & 0xFF;
    }
    void setNutrients(uint8_t nutrients) {
        dormant = (dormant & ~(0xFFULL << NUTRIENTS_BITS)) | (static_cast<uint64_t>(nutrients) << NUTRIENTS_BITS);
    }

    // Get and set the current day
    uint8_t getDay() const {
        return (dormant >> DAY_BITS) & 0xFF;
    }
    void setDay(uint8_t day) {
        dormant = (dormant & ~(0xFFULL << DAY_BITS)) | (static_cast<uint64_t>(day) << DAY_BITS);
    }

    // Get and set Depth
    uint16_t getDepth() const {
        return (seed >> DEPTH_BITS) & 0xFFFF;
    }
    void setDepth(uint16_t depth) {
        seed = (seed & ~(0xFFFFULL << DEPTH_BITS)) | (static_cast<uint64_t>(depth) << DEPTH_BITS);
    }

    // Get and set Actions
    uint16_t getActions() const {
        return (sm >> ACTION_BITS) & 0xFFFF;
    }
    void setActions(uint16_t actions) {
        sm = (sm & ~(0xFFFFULL << ACTION_BITS)) | (static_cast<uint64_t>(actions) << ACTION_BITS);
    }
};


#define TREES 0x1FFFFFFFFFULL // First 37 bits mask

// Get player's trees
#define getPlayerSeeds(s, pId) ((s.seed & TREES) & (s.me[pId]))
#define getPlayerTrees1(s, pId) ((s.sm & TREES) & (s.me[pId]))
#define getPlayerTrees2(s, pId) ((s.md & TREES) & (s.me[pId]))
#define getPlayerTrees3(s, pId) ((s.ta & TREES) & (s.me[pId]))

// Get active trees (non-dormant trees)
#define getPlayersActiveSeeds(s, pId) ((~s.dormant) & (s.me[pId]) & TREES & s.seed)
#define getPlayersActiveTrees1(s, pId) ((~s.dormant) & (s.me[pId]) & TREES & s.sm)
#define getPlayersActiveTrees2(s, pId) ((~s.dormant) & (s.me[pId]) & TREES & s.md)
#define getPlayersActiveTrees3(s, pId) ((~s.dormant) & (s.me[pId]) & TREES & s.ta)

// Count player's trees
#define cntPlayersSeeds(s, pId) cntBits((s.seed & TREES) & (s.me[pId]))
#define cntPlayersTrees1(s, pId) cntBits((s.sm & TREES) & (s.me[pId]))
#define cntPlayersTrees2(s, pId) cntBits((s.md & TREES) & (s.me[pId]))
#define cntPlayersTrees3(s, pId) cntBits((s.ta & TREES) & (s.me[pId]))

#define cntPlayerTrees(s, sz, p) cntBits((TREES & (sz == 0 ? s.seed : (sz == 1 ? s.sm : (sz == 2 ? s.md : s.ta))) & s.me[p]))

// Get locations where seeds can be planted
#define canSeedLocations(s) ((~(s.seed | s.sm | s.md | s.ta | SOIL_ZERO)) & TREES)


std::vector<int> getSetBitPositions(uint64_t num) {
    std::vector<int> positions;
    num &= (1ULL << 37) - 1; // Mask to get only the first 37 bits

    while (num) {
        uint64_t leastSignificantBit = num & -num; // Isolate lowest set bit
        positions.push_back(__builtin_ctzll(leastSignificantBit));
        num &= (num - 1); // Clear the lowest set bit
    }
    return positions;
}


uint64_t getShadowCells(State &state, int day) {
    day = day % 6;
    uint64_t shadowCells = 0;

    // for (int i : getSetBitPositions(state.sm)) {
    //     shadowCells |= SHADOW_CELLS_MASK[i][1][day];
    // }
    // for (int i : getSetBitPositions(state.md)) {
    //     shadowCells |= SHADOW_CELLS_MASK[i][2][day];
    // }
    // for (int i : getSetBitPositions(state.ta)) {
    //     shadowCells |= SHADOW_CELLS_MASK[i][3][day];
    // }

    for (int i = 0; i < N; i++) {
        if (state.hasTreeSize1(i)) {
            shadowCells |= SHADOW_CELLS_MASK[i][1][day];
        } else if (state.hasTreeSize2(i)) {
            shadowCells |= SHADOW_CELLS_MASK[i][2][day];
        } else if (state.hasTreeSize3(i)) {
            shadowCells |= SHADOW_CELLS_MASK[i][3][day];
        }
    }
    return shadowCells;
}


vector<vector<int>> grid;

class Game {
public:
    State state;

    void initPrintMapping() {
        int cur = 0;
        // move in direction 1 or 2 or 3 until possible -> will reach top left corner
        while (true) {
            if (NEIGH[cur][1] != -1) {
                cur = NEIGH[cur][1];
            } else if (NEIGH[cur][2] != -1) {
                cur = NEIGH[cur][2];
            } else if (NEIGH[cur][3] != -1) {
                cur = NEIGH[cur][3];
            } else {
                break;
            }
        }

        grid.clear();
        vector<int> row;
        while (cur != -1) {
            row.push_back(cur);
            cur = NEIGH[cur][0];
        }
        grid.push_back(row);
        row.clear();

        while (grid.size() < 7) {
            cur = grid.back()[0];
            if (NEIGH[cur][4] != -1) {
                cur = NEIGH[cur][4];
            } else {
                cur = NEIGH[cur][5];
            }
            while (cur != -1) {
                row.push_back(cur);
                cur = NEIGH[cur][0];
            }
            grid.push_back(row);
            row.clear();
        }
    }

    void printGrid() {
        cerr << "-------------------------" << endl;
        for (int i = 0; i < grid.size(); i++) {
            for (int j = 0; j < 3-abs(3-i); j++) cerr << " ";
            for (int j = 0; j < abs(3-i); j++) cerr << "- ";

            for (int j = 0; j < grid[i].size(); j++) {
                int index = grid[i][j];
                string cell;
                if (state.isPlayersTree(index, ME)) {
                    if (state.hasSeed(index)) cell = "A";
                    else if (state.hasTreeSize1(index)) cell = "B";
                    else if (state.hasTreeSize2(index)) cell = "C";
                    else cell = "D";
                } else if (state.isPlayersTree(index, HE)) {
                    if (state.hasSeed(index)) cell = "a";
                    else if (state.hasTreeSize1(index)) cell = "b";
                    else if (state.hasTreeSize2(index)) cell = "c";
                    else cell = "d";
                } else {
                    cell = "*";
                }
                cerr << cell << " ";
            }

            for (int j = 0; j < abs(3-i); j++) cerr << "- ";
            cerr << endl;
        }
        cerr << "-------------------------" << endl;
    }

    string toBinary(uint64_t a) {
        string ans = "";
        for (int i = 0; i < N; i++) {
            ans += ((a >> i) & 1ULL) ? '1' : '0';
        }
        return ans;
    }

    void printTreesInBinary() {
        cerr << "----------" << endl;
        cerr << toBinary(state.seed) << endl;
        cerr << toBinary(state.sm) << endl;
        cerr << toBinary(state.md) << endl;
        cerr << toBinary(state.ta) << endl;
        cerr << toBinary(state.me[0]) << endl;
        cerr << toBinary(state.me[1]) << endl;
        cerr << toBinary(state.dormant) << endl;
        cerr << "----------" << endl;
    }

    // read and initialise the richness and neighours
    void init() {
        state = State();
        SOIL_ZERO = 0;

        int numberOfCells; // always will be 37
        cin >> numberOfCells; cin.ignore();

        for (int i = 0; i < numberOfCells; i++) {
            int index;
            cin >> index;
            cin >> RICHNESS[index];
            cin >> NEIGH[index][0] >> NEIGH[index][1] >> NEIGH[index][2] >> NEIGH[index][3] >> NEIGH[index][4] >> NEIGH[index][5];
            cin.ignore();

            if (RICHNESS[index] == 0) {
                SOIL_ZERO |= 1ULL << index;
            }
        }

        // initialize rechable cells for each cell and each depth
        for (int i = 0; i < N; i++) {
            for (int depth = 1; depth <= 3; depth++) {
                REACHABLE_CELLS[i][depth].clear();
                REACHABLE_CELLS_MASK[i][depth] = 0;
                int vis[37] = {0};
                queue<int> q;
                q.push(i);
                vis[i] = 1;

                int dep = depth;
                while (!q.empty() && dep > 0) {
                    int sz = q.size();
                    while (sz--) {
                        int u = q.front(); q.pop();
                        for (int dir = 0; dir < 6; dir++) {
                            int v = NEIGH[u][dir];
                            if (v != -1 && !vis[v]) {
                                q.push(v);
                                vis[v] = 1;
                                REACHABLE_CELLS[i][depth].push_back(v);
                                REACHABLE_CELLS_MASK[i][depth] |= 1ULL << v;
                            }
                        }
                    }
                    dep--;
                }
            }
        }

        // cerr << "Reachable cells for each cell up to depth 3 : " << endl;
        // for (int i = 0; i < N; i++) {
        //     cerr << i << " :" << endl;;
        //     cerr << " 1 : ";
        //     for (int j : REACHABLE_CELLS[i][1]) {
        //         cerr << j << " ";
        //     }
        //     cerr << endl;
        //     cerr << " 2 : ";
        //     for (int j : REACHABLE_CELLS[i][2]) {
        //         cerr << j << " ";
        //     }
        //     cerr << endl;
        //     cerr << " 3 : ";
        //     for (int j : REACHABLE_CELLS[i][3]) {
        //         cerr << j << " ";
        //     }
        //     cerr << endl;
        // }
        // cerr << endl;

        // initialize shadow cells fro each cell and each tree size
        for (int i = 0; i < N; i++) {
            for (int sz = 1; sz <= 3; sz++) {
                for (int day = 0; day < 6; day++) {
                    SHADOW_CELLS[i][sz][day].clear();
                    SHADOW_CELLS_MASK[i][sz][day] = 0;
                    int shadowDirection = getShadowDirection(day);
                    int cur = i, curSize = sz;
                    while (curSize > 0) {
                        cur = NEIGH[cur][shadowDirection];
                        if (cur < 0) { // board edge found
                            break;
                        }
                        SHADOW_CELLS[i][sz][day].push_back(cur);
                        SHADOW_CELLS_MASK[i][sz][day] |= 1ULL << cur;
                        curSize--;
                    }
                }

            }
        }
    }

    void read() {
        state.clear();

        int day, nutrients, mySunPoints, hisSunPoints, myScore, hisScore, oppWaiting;
        cin >> day; cin.ignore();
        cin >> nutrients; cin.ignore();
        cin >> mySunPoints >> myScore; cin.ignore();
        cin >> hisSunPoints >> hisScore >> oppWaiting; cin.ignore();

        state.setDay(day);
        state.setNutrients(nutrients);
        state.setPlayersSun(ME, mySunPoints);
        state.setPlayersSun(HE, hisSunPoints);
        state.setPlayersScore(ME, myScore);
        state.setPlayersScore(HE, hisScore);

        // TODO: use oppWaiting somehow
        state.playerWaiting[HE] = oppWaiting;

        int numberOfTrees;
        cin >> numberOfTrees; cin.ignore();

        for (int i = 0; i < numberOfTrees; i++) {
            int index; // location of this tree
            int treeSize; // size of this tree: 0-3
            bool isMine; // 1 if this is your tree
            bool isDormant; // 1 if this tree is dormant
            cin >> index >> treeSize >> isMine >> isDormant; cin.ignore();

            if (treeSize == 0) {
                state.setSeed(index, 1);
            } else if (treeSize == 1) {
                state.setTreeSize1(index, 1);
            } else if (treeSize == 2) {
                state.setTreeSize2(index, 2);
            } else {
                state.setTreeSize3(index, 3);
            }

            state.setPlayersTree(index, (isMine ? ME : HE), 1);
            state.setPlayersTree(index, (isMine ? HE : ME), 0);

            state.setDormant(index, isDormant);
        }

        int numberOfPossibleActions; // all legal actions
        cin >> numberOfPossibleActions; cin.ignore();
        for (int i = 0; i < numberOfPossibleActions; i++) {
            string possibleAction;
            getline(cin, possibleAction); // try printing something from here to start with
            // cerr << possibleAction << " ";
        }
        // cerr << endl;        
    }

    vector<Action> getAllActions(int pId) {
        /*
            last day = 23
            Rules:
                1) Never complete before 11th day
                2) No complete between days 11 to 18 and size 3 trees are less than 3
                2) No wait if seed is possible
                3) atmost 4 seeds
                4) Never try seed on a cell next to MY trees
        */

        // opponent always waits
        if (pId == HE) {
            return {Action(WAIT)};
        }
        
        bool shouldSeed = state.getDay() < 20 && cntPlayersSeeds(state, pId) < 4;
        bool shouldGrow0 = state.getDay() < 21;
        bool shouldGrow1 = state.getDay() < 22;
        bool shouldGrow2 = state.getDay() < 23;
        bool shouldComplete = (state.getDay() > 11) && (state.getDay() > 18 || cntPlayersTrees3(state, pId) > 2);

        bool shouldWait = 1;

        // int howMuchTrees[5] = {3, 4, 5, 6}; // seeds, size 1, size 2, size 3 trees

        vector<Action> completeActions, growActions, seedActions;
        uint64_t trees; 

        // COMPLETE actions
        if (shouldComplete && state.getPlayersSun(pId) >= completionCost()) {
            trees = getPlayersActiveTrees3(state, pId);
            for (int i = 0; i < N; i++) {
                if ((trees >> i) & 1ULL) {
                    completeActions.push_back(Action(COMPLETE, i));
                }
            }
        }

        // GROW a size 2 tree actions
        if (shouldGrow2 && state.getPlayersSun(pId) >= growCost(2, cntPlayersTrees3(state, pId))) {
            trees = getPlayersActiveTrees2(state, pId);
            for (int i = 0; i < N; i++) {
                if (((trees >> i) & 1ULL)) {
                    growActions.push_back(Action(GROW, i));
                }
            }
        }

        // GROW a size 1 tree actions
        if (shouldGrow1 && state.getPlayersSun(pId) >= growCost(1, cntPlayersTrees2(state, pId))) {
            trees = getPlayersActiveTrees1(state, pId);
            for (int i = 0; i < N; i++) {
                if (((trees >> i) & 1ULL)) {
                    growActions.push_back(Action(GROW, i));
                }
            }
        }

        // GROW a size 0 tree actions
        if (shouldGrow0 && state.getPlayersSun(pId) >= growCost(0, cntPlayersTrees1(state, pId))) {
            trees = getPlayersActiveSeeds(state, pId);
            for (int i = 0; i < N; i++) {
                if (((trees >> i) & 1ULL)) {
                    growActions.push_back(Action(GROW, i));
                }
            }
        }

        // SEEDING actions
        if (shouldSeed && state.getPlayersSun(pId) >= seedingCost(cntPlayersSeeds(state, pId))) {
            uint64_t seedingLocations = canSeedLocations(state);
            uint64_t trees1 = getPlayersActiveTrees1(state, pId);
            uint64_t trees2 = getPlayersActiveTrees2(state, pId);
            uint64_t trees3 = getPlayersActiveTrees3(state, pId);
            uint64_t allTrees = TREES & state.sm & state.md & state.ta & state.me[pId];
            for (int i = 0; i < N; i++) {
                // if (cntPlayersSeeds(state, pId) < 2) {
                //     if ((trees1 >> i) & 1ULL) { // seeding from size 1 tree
                //         for (int j : REACHABLE_CELLS[i][1]) {
                //             if ((seedingLocations >> j) & 1ULL) {
                //                 if ((REACHABLE_CELLS_MASK[j][1] & allTrees) == 0) { // not my tree in neigh
                //                     seedActions.push_back(Action(SEED, i, j));
                //                 }
                //             }
                //         }
                //     }
                // }
                if ((trees2 >> i) & 1ULL) { // seeding from size 2 tree
                    for (int j : REACHABLE_CELLS[i][2]) {
                        if ((seedingLocations >> j) & 1ULL) {
                            bool no = 0;
                            for (int k : REACHABLE_CELLS[j][1]) {
                                if (state.isPlayersTree(k, pId)) {
                                    no = 1;
                                    break;
                                }
                            }
                            if (!no) { // not my tree in neigh
                                seedActions.push_back(Action(SEED, i, j));
                            }
                        }
                    }
                }
                if ((trees3 >> i) & 1ULL) { // seeding from size 3 tree
                    for (int j : REACHABLE_CELLS[i][3]) {
                        if ((seedingLocations >> j) & 1ULL) {
                            bool no = 0;
                            for (int k : REACHABLE_CELLS[j][1]) {
                                if (state.isPlayersTree(k, pId)) {
                                    no = 1;
                                    break;
                                }
                            }
                            if (!no) { // not my tree in neigh
                                seedActions.push_back(Action(SEED, i, j));
                            }
                        }
                    }
                }
            }
        }

        sort(completeActions.begin(), completeActions.end(), [&](const Action& a, const Action &b){
            return RICHNESS[a.getIndex2()] < RICHNESS[b.getIndex2()];
        });
        sort(growActions.begin(), growActions.end(), [&](const Action& a, const Action &b){
            return RICHNESS[a.getIndex2()] > RICHNESS[b.getIndex2()];
        });
        sort(seedActions.begin(), seedActions.end(), [&](const Action& a, const Action &b){
            return RICHNESS[a.getIndex2()] > RICHNESS[b.getIndex2()];
        });

        vector<Action> actions;
        for (auto &a : completeActions) {
            actions.push_back(a);
        }
        for (auto &a : growActions) {
            actions.push_back(a);
        }
        for (auto &a : seedActions) {
            actions.push_back(a);
        }

        // WAIT action
        if (seedActions.empty()) {
            actions.push_back(Action(WAIT));
        }

        return actions;
    }

    void doAction (Action &action, Action &lastAction, int pId) {
        if (action.getType() == SEED) {
            if (lastAction.getType() == SEED && action.getIndex2() == lastAction.getIndex2()) { // both player tries to SEED on same cell
                state.setDormant(action.getIndex1(), 1);
                return;
            }

            // first update sun points as aother informations will be changed after
            state.setPlayersSun(pId, state.getPlayersSun(pId) - seedingCost(cntPlayersSeeds(state, pId)));

            int u = action.getIndex1(), v = action.getIndex2();
            state.setDormant(u, 1);
            state.setDormant(v, 1);
            state.setSeed(v, 1);
            state.setPlayersTree(v, pId, 1);
            state.setPlayersTree(v, otherPId(pId), 0);
        }
        else if (action.getType() == GROW) {
            int u = action.getIndex1();
            if (state.hasSeed(u)) { // Growing from 0 -> 1
                state.setPlayersSun(pId, state.getPlayersSun(pId) - growCost(0, cntPlayersTrees1(state, pId)));
                state.setSeed(u, 0);
                state.setTreeSize1(u, 1);
            }
            else if (state.hasTreeSize1(u)) { // growing from 1 -> 2
                state.setPlayersSun(pId, state.getPlayersSun(pId) - growCost(1, cntPlayersTrees2(state, pId)));
                state.setTreeSize1(u, 0);
                state.setTreeSize2(u, 1);
            }
            else { // last, growing from 2 -> 3
                state.setPlayersSun(pId, state.getPlayersSun(pId) - growCost(2, cntPlayersTrees3(state, pId)));
                state.setTreeSize2(u, 0);
                state.setTreeSize3(u, 1);
            }
            state.setDormant(u, 1);
        }
        else if (action.getType() == COMPLETE) {
            int u = action.getIndex1();
            state.setPlayersSun(pId, state.getPlayersSun(pId) - completionCost());
            
            int scoreToAdd = getCompletionBonusScore(u) + state.getNutrients();
            if (lastAction.getType() == COMPLETE) { // if both players make COMPLETE move, both players get full nutrients socre and then nutrents get decreased by 2
                scoreToAdd += 1;
            }

            state.setNutrients(state.getNutrients() - 1);
            state.setPlayersScore(pId, state.getPlayersScore(pId) + scoreToAdd);
            state.setTreeSize3(u, 0);
            state.setPlayersTree(u, pId, 0);
        }
        else if (action.getType() == WAIT) {
            if (lastAction.getType() == WAIT) {
                changeDay();
            } else {
                state.playerWaiting[pId] = 1;
            }
        }
    }

    void changeDay() {
        state.setDay(state.getDay() + 1); // increment day

        for (int i = 0; i < N; i++) { // setIsDormant false for all trees;
            state.setDormant(i, 0);
        }

        uint64_t shadow = getShadowCells(state, state.getDay() % 6);

        int sunToAdd[2] = {0};
        for (int i = 0; i < N; i++) { // give sun points as players get sun points at the start of each day
            if (((shadow >> i) & 1ULL) == 0) {
                int p = state.isPlayersTree(i, ME) ? 0 : 1;
                if (state.hasTreeSize1(i)) sunToAdd[p] += 1;
                else if (state.hasTreeSize2(i)) sunToAdd[p] += 2;
                else if (state.hasTreeSize3(i)) sunToAdd[p] += 3;
            }
        }

        state.setPlayersSun(ME, state.getPlayersSun(ME) + sunToAdd[ME]);
        state.setPlayersSun(HE, state.getPlayersSun(HE) + sunToAdd[HE]);

        state.playerWaiting[ME] = 0;
        state.playerWaiting[HE] = 0;
    }

    double eval(int pId, Action lastAction) {
        /*
            1) Points + (0.33+0.01*remainingDay) * Suns, taking mine and subtracting the opponent score
            2) Simulating the whole game until the end with both players waiting to count the suns at the end of 
                the game. Taking factor * (mySunsPerDay - opponentSunsPerDay) with a factor starting at 0.7 and 
                getting multiplied by 0.98 to further decrease over time.
            3) -1 for each pair of my own trees that can possibly give shadow to each other when reaching size 3
            4) A bonus for having trees of a certain size. Size 1 is worth 1/11, size 2 4/11 and size 3 is 11/11. 
                These are equal to the costs of growing a tree from size 0 to size 3 without any other trees. 
                I multiply these by (Nutrient - 0.5) * Min(1, remainingDays * 0.09) as the motivation to keep a 
                tree will drop over time. And a multiplier on top, that penalizes having too many trees of 
                the same size.
        */

        // double evaluation = 0;

        // uint64_t shadow[6];
        // for (int d = 0; d < 6; d++) {
        //     shadow[d] = getShadowCells(state, d);
        // }

        // int sun[2] = {0}; // sun points gathered until the game ends
        // double factor = 0.7;
        // for (int day = state.getDay(); day < 24; day++) {
        //     for (int i = 0; i < N; i++) {
        //         if (((shadow[day % 6] >> i) & 1ULL) == 0ULL) {
        //             if (state.hasTreeSize1(i)) {
        //                 sun[state.isPlayersTree(i, pId) ? pId : otherPId(pId)] += factor * 1.0;
        //             } else if (state.hasTreeSize2(i)) {
        //                 sun[state.isPlayersTree(i, pId) ? pId : otherPId(pId)] += factor * 2.0;
        //             } else if (state.hasTreeSize3(i)) {
        //                 sun[state.isPlayersTree(i, pId) ? pId : otherPId(pId)] += factor * 3.0;
        //             }
        //         }
        //     }
        //     factor *= 0.95;
        // }

        // int shadowPairs = 0; // pairs of my trees with can possibly shadow each other on reaching size 3
        // for (int i = 0; i < N; i++) {
        //     if (state.isPlayersTree(i, pId) && (state.hasTree(i) || state.hasSeed(i))) {
        //         for (int xx = 0, day = state.getDay(); xx < 6 && day < 24; xx++, day++) {
        //             for (int j : SHADOW_CELLS[i][3][day % 6]) {
        //                 shadowPairs++;
        //             }
        //         }
        //     }
        // }

        // int remainingDays = 24 - state.getDay();

        // evaluation += state.getPlayersScore(pId) - state.getPlayersScore(otherPId(pId));
        // evaluation += (0.33 + 0.01 * remainingDays) * (state.getPlayersSun(pId) - state.getPlayersSun(otherPId(pId)));
        // evaluation += sun[pId] - sun[otherPId(pId)];
        // evaluation += ((1.0/11.0 * cntPlayersTrees1(state, pId)) + 
        //                 (4.0/11.0 * cntPlayersTrees2(state, pId)) + 
        //                 (11.0/11.0 * cntPlayersTrees3(state, pId))) * (state.getNutrients() - 0.5) * min(1.0, remainingDays * 0.09);
        // evaluation -= shadowPairs;

        // return pId == ME ? evaluation : -evaluation;


        double evaluation = 0;
        double myScore = state.getPlayersScore(pId) + state.getPlayersSun(pId) / 3;
        double hisScore = state.getPlayersScore(otherPId(pId)) + state.getPlayersSun(otherPId(pId)) / 3;

        if (myScore > hisScore) {
            double diff = myScore - hisScore;

            if (diff > 5) {
                evaluation = 1.0 + (diff - 5) * 0.001;
            } else {
                evaluation = 0.5 + 0.5 * diff / 5;
            }
        } else if (myScore < hisScore) {
            double diff = hisScore - myScore;

            if (diff > 5) {
                evaluation = -1.0 - (diff - 5) * 0.001;
            } else {
                evaluation = -0.5 - 0.5 * diff / 5;
            }
        } else {
            int myNumberOfTrees = cntPlayersTrees1(state, pId) + cntPlayersTrees2(state, pId) + cntPlayersTrees3(state, pId);
            int hisNumberOfTrees = cntPlayersTrees1(state, otherPId(pId)) + cntPlayersTrees2(state, otherPId(pId)) + cntPlayersTrees3(state, otherPId(pId));
            if (myNumberOfTrees > hisNumberOfTrees) {
                evaluation = 0.25 + myScore * 0.001;
            } else if (myNumberOfTrees < hisNumberOfTrees) {
                evaluation = -0.25 + myScore * 0.001;
            } else {
                evaluation = myScore * 0.001;
            }
        }

        return evaluation;
    }
};


string getCommandFromAction (Action action) {
    stringstream ss;
    
    if (action.getType() == SEED) {
        ss << "SEED " << action.getIndex1() << " " << action.getIndex2();
    } else if (action.getType() == GROW) {
        ss << "GROW " << action.getIndex1();
    } else if (action.getType() == COMPLETE) {
        ss << "COMPLETE " << action.getIndex1();
    } else {
        ss << "WAIT";
    }

    return ss.str();
}


// Action minmax (int depth, Game game, int pId, double &score, Action lastAction) {
//     // if both players waiting, change day
//     if (game.state.playerWaiting[ME] && game.state.playerWaiting[HE]) {
//         game.changeDay();
//         lastAction = Action();
//     }

//     if (game.state.getDay() > 24) {
//         return Action();
//     }

//     vector<Action> actions = game.getAllActions(pId);

//     if (depth <= 0) {
//         // Action action = actions[rand() % (int)actions.size()];
//         Action action = actions[0];
//         game.doAction(action, lastAction, pId);
//         score = game.eval(pId, lastAction);
//         return action;
//     }


//     double curScore;
//     Action bestAction;

//     if (pId == ME) {
//         curScore = -1e9;
//         for (auto &action : actions) {
//             Game g = game;
//             g.doAction(action, lastAction, pId);
            
//             double s;
//             minmax(depth - 1, g, otherPId(pId), s, action);
//             if (s > curScore) {
//                 curScore = s;
//                 bestAction = action;
//             }
//         }
//     } else {
//         curScore = 1e9;
//         for (auto &action : actions) {
//             Game g = game;
//             g.doAction(action, lastAction, pId);
            
//             double s;
//             minmax(depth - 1, g, otherPId(pId), s, action);
//             if (s < curScore) {
//                 curScore = s;
//                 bestAction = action;
//             }
//         }
//     }

//     score = curScore;
//     return bestAction;
// }

// int simulations = 0;
// double minmaxAlphaBeta(Game game, int depth, int pId, double alpha, double beta, Action &lastAction) {
//     simulations++;
//     if (depth == 0 || game.state.getDay() == 24) {
//         return game.eval(pId, lastAction);
//     }

//     vector<Action> actions = game.getAllActions(pId);

//     if (pId == ME) {
//         double maxEval = -100000;
//         for (Action& action : actions) {
//             Game newGame = game;

//             // cerr << "Aefore action : " << getCommandFromAction(action) << endl;
//             // newGame.printGrid();
//             newGame.doAction(action, lastAction, pId);
//             // cerr << "After action : " << getCommandFromAction(action) << endl;
//             // newGame.printGrid();

//             double eval = minmaxAlphaBeta(newGame, depth - 1, otherPId(pId), alpha, beta, action);
//             maxEval = max(maxEval, eval);
//             alpha = max(alpha, eval);
//             if (beta <= alpha) {
//                 break; // Beta cut-off
//             }
//         }
//         return maxEval;
//     } else {
//         double minEval = 100000;
//         for (Action& action : actions) {
//             Game newGame = game;

//             // cerr << "Before action : " << getCommandFromAction(action) << endl;
//             // newGame.printGrid();
//             newGame.doAction(action, lastAction, pId);
//             // cerr << "After action : " << getCommandFromAction(action) << endl;
//             // newGame.printGrid();
            
//             double eval = minmaxAlphaBeta(newGame, depth - 1, otherPId(pId), alpha, beta, action);
//             minEval = min(minEval, eval);
//             beta = min(beta, eval);
//             if (beta <= alpha) {
//                 break; // Alpha cut-off
//             }
//         }
//         return minEval;
//     }
// }

int simulations = 0;
// opponent always waiting (lastAction = Waiting action)
double recur(Game game, int depth, Action &lastAction) {
    simulations++;
    if (depth == 0 || game.state.getDay() == 24) {
        return game.eval(ME, lastAction);
    }

    vector<Action> actions = game.getAllActions(ME);

    double maxEval = -1e9;
    for (Action &action : actions) {
        Game newGame = game;
        newGame.doAction(action, lastAction, ME);
        double eval = recur(newGame, depth - 1, lastAction);
        maxEval = max(maxEval, eval);
    }
    return maxEval;
}

Action findBestMove(Game& game, int depth) {
    double maxEval = -100000;
    Action bestAction = Action(), lastAction = Action();

    vector<Action> actions = game.getAllActions(ME);
    for (Action& action : actions) {
        Game newGame = game;

        // cerr << "Aefore action : " << getCommandFromAction(action) << endl;
        // newGame.printGrid();
        newGame.doAction(action, lastAction, ME);
        // cerr << "After action : " << getCommandFromAction(action) << endl;
        // newGame.printGrid();

        // double eval = minmaxAlphaBeta(newGame, depth - 1, HE, -100000, 100000, action);

        lastAction = Action(WAIT);
        double eval = recur(newGame, depth - 1, lastAction);
        if (eval > maxEval) {
            maxEval = eval;
            bestAction = action;
        }
    }
    return bestAction;
}


int main() {
    srand(time(NULL));

    Game game;
    game.init();
    game.initPrintMapping();
    int first = 1;

    // game loop
    while (1) {
        simulations = 0;

        game.read();
        game.printGrid();
        // game.printTreesInBinary();

        if (first) {
            vector<Action> actions = game.getAllActions(ME);
            // first = 0;
            cerr << endl;
            cerr << actions.size() << endl;
            for (auto action : actions) {
                cerr << getCommandFromAction(action) << endl;
            }
            cerr << endl;
        }

        int depth = 6;
        double score;
        Action lastAction = Action();
        // Action action = minmax(depth, game, ME, score, lastAction);
        // Action action = findBestMove(game, depth);
        Action action = game.getAllActions(ME)[0];

        cout << getCommandFromAction(action) << " " << simulations << endl;

        // break;
        // GROW cellIdx | SEED sourceIdx targetIdx | COMPLETE cellIdx | WAIT <message>
    }
    return 0;
}













