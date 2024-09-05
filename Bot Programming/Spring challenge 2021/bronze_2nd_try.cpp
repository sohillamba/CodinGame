/*
    Inspired by: https://www.codingame.com/forum/t/spring-challenge-2021-feedbacks-strategies/190849/2
    user: (eulerscheZahl) https://forum.codingame.com/u/eulerscheZahl

    And: https://www.codingame.com/forum/t/spring-challenge-2021-feedbacks-strategies/190849/32
    user: (Kodle) https://forum.codingame.com/u/Kodle

    tried to implement minmax algo
*/


#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <cstring>
#include <sstream>
#include <set>

using namespace std;

const int N = 37;
const int ME = 1;
const int HE = 0;

const int GROW = 0;
const int SEED = 1;
const int COMPLETE = 2;
const int WAIT = 3;


int neigh[37][6];

int otherPId (int pId) {
    return (pId == ME) ? HE : ME;
}

vector<int> getReachableCellIds(int startCellId, int depth) {
    int vis[37] = {0};
    vector<int> ans;
    queue<int> q;
    q.push(startCellId);
    vis[startCellId] = 1;
    while (!q.empty() && depth > 0) {
        int sz = q.size();
        while (sz--) {
            int u = q.front(); q.pop();
            for (int i = 0; i < 6; i++) {
                int v = neigh[u][i];
                if (v != -1 && !vis[v]) {
                    q.push(v);
                    ans.push_back(v);
                }
            }
        }
        depth--;
    }
    return ans;
}


int seedingCost(int *treeCount) {
    return treeCount[0];
}

int growCost(int treeSize, int *treeCount) { // tree size and the count of trees of (array of 4 ([0-3] size tree count))
    if (treeSize == 0) {        // seed to small tree
        return 1 + treeCount[1];
    } else if (treeSize == 1) { // small to medium tree
        return 3 + treeCount[2];
    } else {                    // medium to large tree
        return 7 + treeCount[3];
    }
}

int completionCost() {
    return 4;
}


/*
directions:
* * * * * *
* * * * * *
 * * 2 1 * *
* * 3 # 0 *
 * * 4 5 * *
* * * * * *
*/


int getSunDirection (int day) {
    return day % 6;
}

int getShadowDirection (int day) {
    // return (day + 3) % 6;
    return (getSunDirection(day) + 3) % 6;
}



int getRichness(int cell) {
    return cell & 3;
}
void setRichness(int &cell, int val) {
    cell &= 0b1111100; // clear richness bits
    cell |= val;
}

bool isTree(int cell) {
    return (cell >> 2) & 1;
}
void setIsTree(int &cell, int isTree) {
    cell &= 0b1111011; // clear isTree bits
    cell |= isTree << 2;    // set isTree bits
}

int getTreeSize(int cell) {
    return (cell >> 3) & 3;
}
void setTreeSize(int &cell, int size) {
    cell &= 0b1100111; // clear size bits
    cell |= size << 3;
}

int getTreeOwner(int cell) {
    return (cell >> 5) & 1;
}
void setTreeOwner(int &cell, int owner) {
    cell &= 0b1011111; // clear owner bits
    cell |= owner << 5;
}

bool isDormant(int cell) {
    return (cell >> 6) & 1;
}
void setIsDormant(int &cell, int isDormant) {
    cell &= 0b0111111;
    cell |= isDormant << 6;
}


// struct Cell {
//     int richness;
//     bool isTree;
//     int treeSize;
//     int treeOwner;
//     bool isDormant;
// };


class Action {
public:
    int type;
    int index1;
    int index2;     // only in case of seeding (target index)

    Action () {
        type = index1 = index2 = -1;
    }

    Action (int t) {
        type = t;
    }

    Action (int t, int id) {
        type = t;
        index1 = id;
    }

    Action (int t, int id1, int id2) {
        type = t;
        index1 = id1;
        index2 = id2;
    }
};

class State {
public:
    int cells[N];   // bit   0-1 = richness         (0-3)
                    // bit     2 = is tree on cell  (1=yes, 0=no)
                    // bit   3-4 = tree size        (1=yes, 0=no)
                    // bit     5 = tree owner       (1=me, 0=he)
                    // bit     6 = is dormant       (1=yes, 0=no)


    // // using 13 bits for sun points
    // // using 13 bits for score
    // int data;       // bit  0-12 = my sun points    
    //                 // bit   0-4 = day              (24 days so 5 bits are required)
    //                 // bit   5-9 = nutrients        (20 max value so need 5 bits)
    //                 // bit   

    int day;
    int nutrients;

    int sunPoints[2];
    int score[2];
    bool playerWaiting[2];

    int treeCount[2][4];    // 2 for player ids, 4 for tree sizes
    int treeIds[2][4][37];  // 2 for player ids, 4 for tree sizes

    int getCompletionBonusScore (int index) {
        int r = getRichness(cells[index]);
        if (r == 1) {
            return 0;
        } else if (r == 2) {
            return 2;
        } else if (r == 3) {
            return 4;
        } else {
            return 0;
        }
    }

    // read and initialise the richness and neighours
    void init() {
        for (int i = 0; i < N; i++) {
            cells[i] = 0; // no tree nothing
        }
        int numberOfCells; // always will be 37
        cin >> numberOfCells; cin.ignore();

        for (int i = 0; i < numberOfCells; i++) {
            int index, richness;
            cin >> index >> richness;
            cin >> neigh[index][0] >> neigh[index][1] >> neigh[index][2] >> neigh[index][3] >> neigh[index][4] >> neigh[index][5];
            cin.ignore();
            setRichness(cells[index], richness);
            setIsTree(cells[index], false);
            setIsDormant(cells[index], false);
        }
        playerWaiting[ME] = 0;
        memset(treeCount, 0, sizeof treeCount);
    }

    void read() {
        cin >> day; cin.ignore();
        cin >> nutrients; cin.ignore();
        cin >> sunPoints[ME] >> score[ME]; cin.ignore();
        cin >> sunPoints[HE] >> score[HE] >> playerWaiting[HE]; cin.ignore();

        int numberOfTrees;
        cin >> numberOfTrees; cin.ignore();

        memset(treeCount, 0, sizeof treeCount);

        for (int i = 0; i < numberOfTrees; i++) {
            int cellIndex; // location of this tree
            int treeSize; // size of this tree: 0-3
            bool isMine; // 1 if this is your tree
            bool isDormant; // 1 if this tree is dormant
            cin >> cellIndex >> treeSize >> isMine >> isDormant; cin.ignore();

            setIsTree(cells[cellIndex], 1);
            setTreeSize(cells[cellIndex], treeSize);
            setTreeOwner(cells[cellIndex], isMine);
            setIsDormant(cells[cellIndex], isDormant);

            int &playerTreeCountOfThisSize = treeCount[isMine][treeSize];
            treeIds[isMine][treeSize][playerTreeCountOfThisSize] = cellIndex;
            playerTreeCountOfThisSize++;
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

    /*
        Order:
            First COMPLETE actions
            then GROW action (size 3 > 2 > 1)
            the SEED
        
        Do not SEED from size 1 tree
        Atmost 1 SEED on the map
    */
    vector<Action> getAllActions(int pId) {
        vector<Action> actions;
        if (playerWaiting[pId] == 0) {
            for (int i = 0; i < N; i++) {
                if (isTree(cells[i])) {
                     int sz = getTreeSize(cells[i]);
                     if (sz < 3) { // grow action
                        if (sunPoints[pId] >= growCost(sz, treeCount[pId]) && !isDormant(cells[i])) {
                            actions.push_back(Action(GROW, i));
                        }
                     } else { // complete action
                        if (sunPoints[pId] >= completionCost() && !isDormant(cells[i])) {
                            actions.push_back(Action(COMPLETE, i));
                        }
                     }
                } else if (getRichness(cells[i]) > 0) { // seeding action (source = j, target = i)
                    if (treeCount[pId][0] < 3) { // Atmost 3 SEED on the map
                        for (int j = 0; j < N; j++) {
                            if (isTree(cells[j]) && getTreeOwner(cells[j]) == pId && !isDormant(cells[j])) {
                                if (getTreeSize(cells[j]) > 0) { // Do not SEED from size 0 tree (later on change to size 1)
                                    actions.push_back(Action(SEED, j, i)); // source = j, target = i
                                }
                            }
                        }
                    }
                }
            }
        }

        // COMPLETE -> GROW 2 -> GROW 1 -> GROW 0 -> SEED -> WAIT
        // TODO: more complex logic when type is same
        sort(actions.begin(), actions.end(), [&](const Action &a, const Action &b) {
            if (a.type == COMPLETE && b.type == COMPLETE) {
                return true;
            }
            else if (a.type == COMPLETE || b.type == COMPLETE) {
                return a.type == COMPLETE;
            }
            else if (a.type == GROW && b.type == GROW) {
                return getTreeSize(cells[a.index1]) > getTreeSize(cells[b.index1]);
            }
            else if (a.type == GROW || b.type == GROW) {
                return a.type == GROW;
            }
            else {
                return true;
            }
        });
        actions.push_back(Action(WAIT)); // waiting action
        return actions;
    }

    void doAction (Action &action, Action &lastAction, int pId) {
        if (action.type == SEED) {
            if (lastAction.type == SEED && action.index2 == lastAction.index2) { // both player tries to SEED on same cell
                setIsDormant(cells[action.index1], 1);
                setIsDormant(cells[lastAction.index1], 1);
                return;
            }

            // first update sun points as aother informations will be changed after
            sunPoints[pId] -= seedingCost(treeCount[pId]);

            int u = action.index1, v = action.index2;
            setIsDormant(cells[u], 1);

            setIsDormant(cells[v], 1);
            setIsTree(cells[v], 1);
            setTreeSize(cells[v], 0);
            setTreeOwner(cells[v], pId);

            // update zero size tree info
            treeIds[pId][0][treeCount[pId][0]] = v;
            treeCount[pId][0]++;
        }
        else if (action.type == GROW) {
            int u = action.index1;
            int sz = getTreeSize(cells[u]);
            sunPoints[pId] -= growCost(sz, treeCount[pId]);
            setTreeSize(cells[u], sz + 1);
            setIsDormant(cells[u], 1);

            treeCount[pId][sz]--;
            treeIds[pId][sz+1][treeCount[pId][sz+1]] = u;
            treeCount[pId][sz+1]++;
        }
        else if (action.type == COMPLETE) {
            int u = action.index1;
            sunPoints[pId] -= completionCost();
            
            score[pId] += getCompletionBonusScore(u);
            if (lastAction.type == COMPLETE) {
                score[pId] += nutrients + 1;
            } else {
                score[pId] += nutrients;
            }
            nutrients--;

            setIsTree(cells[u], 0);
            treeCount[pId][3]--;
        }
        else if (action.type == WAIT) {
            playerWaiting[pId] = 1;
        }
    }

    void changeDay() {
        day++; // increment day

        for (int i = 0; i < N; i++) { // setIsDormant false for all trees;
            setIsDormant(cells[i], 0);
        }

        vector<bool> isShadow(N, 0);
        int shadowDirection = getShadowDirection(day);
        for (int i = 0; i < N; i++) {
            if (isTree(cells[i])) {
                int sz = getTreeSize(cells[i]);
                int cur = i;
                while (sz > 0) {
                    cur = neigh[cur][shadowDirection];
                    if (cur < 0) { // board edge found
                        break;
                    }
                    isShadow[cur] = 1;
                    sz--;
                }
            }
        }

        for (int i = 0; i < N; i++) { // give sun points as players get sun points at the start of each day
            if (isTree(cells[i]) && !isShadow[i]) {
                sunPoints[getTreeOwner(cells[i])] += getTreeSize(cells[i]);
            }
        }

        playerWaiting[ME] = 0;
        playerWaiting[HE] = 0;
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

        double evaluation = 0;

        // int isShadow[6][N];
        // memset(isShadow, 0, sizeof isShadow);
        // for (int shadowDirection = 0; shadowDirection < 6; shadowDirection++) {
        //     for (int i = 0; i < N; i++) {
        //         if (isTree(cells[i])) {
        //             int sz = getTreeSize(cells[i]);
        //             int cur = i;
        //             while (sz > 0) {
        //                 cur = neigh[cur][shadowDirection];
        //                 if (cur < 0) { // board edge found
        //                     break;
        //                 }
        //                 isShadow[shadowDirection][cur] = 1;
        //                 sz--;
        //             }
        //         }
        //     }
        // }

        // int sun[2] = {0}; // sun points gathered until the game ends
        // double factor = 0.7;
        // for (int curDay = day; curDay < 24; curDay++) {
        //     int shadowDirection = getShadowDirection(curDay);
        //     for (int i = 0; i < N; i++) {
        //         if (isTree(cells[i]) && !isShadow[shadowDirection][i]) {
        //             sun[getTreeOwner(cells[i])] += factor * getTreeSize(cells[i]);
        //         }
        //     }
        //     factor *= 0.95;
        // }

        int trees[4] = {0}; // count of trees of each size
        int shadowPairs; // pairs of my trees with can possibly shadow each other on reaching size 3
        for (int i = 0; i < N; i++) {
            if (isTree(cells[i])) {
                if (getTreeOwner(cells[i]) == pId) {
                    trees[getTreeSize(cells[i])]++;
                    // for (int dir = 0; dir < 6; dir++) {
                    //     int cur = i, sz = 3;
                    //     while (sz > 0) {
                    //         cur = neigh[cur][dir];
                    //         if (cur < 0) { // board edge found
                    //             break;
                    //         }
                    //         if (isTree(cells[cur]) && getTreeOwner(cells[cur]) == pId) {
                    //             shadowPairs++;
                    //         }
                    //         sz--;
                    //     }
                    // }
                }
            }
        }

        // int remainingDays = 24 - day;

        // evaluation += score[pId] - score[otherPId(pId)];
        // evaluation += (0.33 + 0.01 * remainingDays) * (sunPoints[pId] - sunPoints[otherPId(pId)]);
        // evaluation += sun[pId] - sun[otherPId(pId)];
        // evaluation += ((1.0/11.0 * trees[1]) + (4.0/11.0 * trees[2]) + (11.0/11.0 * trees[3])) * (nutrients - 0.5) * min(1.0, remainingDays * 0.09);

        return trees[0] + 2*trees[1] + 4*trees[2] + 16*trees[3];
        // return evaluation;
    }
};



Action minmax (int depth, State state, int pId, double &score, Action lastAction) {
    vector<Action> actions = state.getAllActions(pId);

    // if both players waiting, change day
    if (state.playerWaiting[ME] && state.playerWaiting[HE]) {
        state.changeDay();
        lastAction = Action();
    }

    if (depth <= 0) {
        Action action = actions[rand() % (int)actions.size()];
        // Action action = actions[0];
        state.doAction(action, lastAction, pId);
        score = state.eval(pId, lastAction);
        return action;
    }


    double curScore;
    Action bestAction;

    if (pId == ME) {
        curScore = -1e9;
        for (auto &action : actions) {
            State g = state;
            g.doAction(action, lastAction, pId);
            
            double s;
            minmax(depth - 1, g, otherPId(pId), s, action);
            if (s > curScore) {
                curScore = s;
                bestAction = action;
            }
        }
    } else {
        curScore = 1e9;
        for (auto &action : actions) {
            State g = state;
            g.doAction(action, lastAction, pId);
            
            double s;
            minmax(depth - 1, g, otherPId(pId), s, action);
            if (s < curScore) {
                curScore = s;
                bestAction = action;
            }
        }
    }

    score = curScore;
    return bestAction;
}




string getCommandFromAction (Action action) {
    stringstream ss;
    
    if (action.type == SEED) {
        ss << "SEED " << action.index1 << " " << action.index2;
    } else if (action.type == GROW) {
        ss << "GROW " << action.index1;
    } else if (action.type == COMPLETE) {
        ss << "COMPLETE " << action.index1;
    } else {
        ss << "WAIT";
    }

    return ss.str();
}


int main() {
    srand(time(NULL));

    State state;
    state.init();
    int first = 1;

    // state loop
    while (1) {
        state.read();

        if (first) {
            // first = 0;
            cerr << endl;
            for (auto &action : state.getAllActions(ME)) {
                cerr << getCommandFromAction(action) << endl;
            }
            cerr << endl;
        }

        int depth = 1;
        double score;
        Action lastAction = Action();
        Action action = minmax(depth, state, ME, score, lastAction);

        cout << getCommandFromAction(action) << endl;

        // break;
        // GROW cellIdx | SEED sourceIdx targetIdx | COMPLETE cellIdx | WAIT <message>
    }
    return 0;
}













