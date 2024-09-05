#include <iostream>
#include <string>
#include <vector>
#include <cstring>

using namespace std;

const int ME = 1;
const int HE = 0;

int treeCount[2][4];    // 2 for player ids, 4 for tree sizes
int treeIds[2][4][37];  // 2 for player ids, 4 for tree sizes

int seedingCost(int seedCount) {
    return seedCount;
}

int growCost(int treeSize, int count) { // tree size and the count of trees of size treeSize + 1
    if (treeSize == 0) {        // seed to small tree
        return 1 + count;
    } else if (treeSize == 1) { // small to medium tree
        return 3 + count;
    } else {                    // medium to large tree
        return 7 + count;
    }
}

int completionCost() {
    return 4;
}

class Cell {
public:
    int richness;
    int treeSize;
    bool owner;
    bool isDormant;

    Cell () {
        treeSize = -1;
        isDormant = false;
    }
};


int main()
{
    int numberOfCells; // 37
    cin >> numberOfCells; cin.ignore();

    vector<Cell> cells(numberOfCells);

    for (int i = 0; i < numberOfCells; i++) {
        int index; // 0 is the center cell, the next cells spiral outwards
        int richness; // 0 if the cell is unusable, 1-3 for usable cells
        int neigh_0; // the index of the neighbouring cell for each direction
        int neigh_1;
        int neigh_2;
        int neigh_3;
        int neigh_4;
        int neigh_5;
        cin >> index >> richness >> neigh_0 >> neigh_1 >> neigh_2 >> neigh_3 >> neigh_4 >> neigh_5; cin.ignore();

        cells[index] = Cell();
        cells[index].richness = richness;
    }

    // game loop
    while (1) {
        int day; // the game lasts 24 days: 0-23
        cin >> day; cin.ignore();

        int nutrients; // the base score you gain from the next COMPLETE action
        cin >> nutrients; cin.ignore();
        
        int sun; // your sun points
        int score; // your current score
        cin >> sun >> score; cin.ignore();
        
        int oppSun; // opponent's sun points
        int oppScore; // opponent's score
        bool oppIsWaiting; // whether your opponent is asleep until the next day
        cin >> oppSun >> oppScore >> oppIsWaiting; cin.ignore();
        
        int numberOfTrees; // the current amount of trees
        cin >> numberOfTrees; cin.ignore();

        memset(treeCount, 0, sizeof treeCount);
        // for (int player = 0; player < 2; player++) {
        //     for (int size = 0; size < 4; size++) {
        //         treeCount[player][size] = 0;
        //     }
        // }

        for (int i = 0; i < numberOfTrees; i++) {
            int cellIndex; // location of this tree
            int treeSize; // size of this tree: 0-3
            bool isMine; // 1 if this is your tree
            bool isDormant; // 1 if this tree is dormant
            cin >> cellIndex >> treeSize >> isMine >> isDormant; cin.ignore();
            
            cells[cellIndex].treeSize = treeSize;
            cells[cellIndex].owner = isMine;
            cells[cellIndex].isDormant = isDormant;

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


        string order = "";
        bool found = 0;

        int &n0 = treeCount[ME][0];
        int &n1 = treeCount[ME][1];
        int &n2 = treeCount[ME][2];
        int &n3 = treeCount[ME][3];

        cerr << n1 << " " << n2 << " " << n3 << endl;

        // if their is only 1 large tree (no seeds, small trees and medium trees) -> use it to plant new seed
        if (n0 + n1 + n2 == 0 && n3 == 1) {
            int cellIndex == treeIds[ME][3][0];
            if (!cells[cellIndex].isDormant) {
                int bestCellId = -1, bestRichness = -1;
                for (int i = 0; i < numberOfCells; i++) { // iterate over all 37 cells for best seed location
                    if (cells[i].treeSize == -1 && cells[i].richness > bestRichness) {
                        bestCellId = i;
                        bestRichness = cells[i].richness;
                    }
                }
                if (bestRichness > 0) {
                    order = "SEED " + to_string(cellIndex) + " " + to_string(bestCellId);
                    found = 1;
                }
            }
        } 

        // if not found then first try completing size 3 tree
        if (!found) {
            if (sun >= completionCost()) {
                for (int i = 0; i < n3; i++) {
                    int cellIndex = treeIds[ME][3][i];
                    if (!cells[cellIndex].isDormant) {
                        order = "COMPLETE " + to_string(cellIndex);
                        found = 1;
                        break;
                    }
                }
            }
        }

        // if no completion possible, try growing size 2 tree into size 3 tree
        if (!found) {
            if (sun >= growCost(2, n3)) {
                for (int i = 0; i < n2; i++) {
                    int cellIndex = treeIds[ME][2][i];
                    if (!cells[cellIndex].isDormant) {
                        order = "GROW " + to_string(cellIndex);
                        found = 1;
                        break;
                    }
                }
            }
        }

        // if still order not possible, try growing size 1 tree into size 2 tree
        if (!found) {
            if (sun >= growCost(1, n2)) {
                for (int i = 0; i < n1; i++) {
                    int cellIndex = treeIds[ME][1][i];
                    if (!cells[cellIndex].isDormant) {
                        order = "GROW " + to_string(cellIndex);
                        found = 1;
                        break;
                    }
                }
            }
        }

        // if still order not possible, try growing seed into size 1 tree
        if (!found) {
            if (sun >= growCost(0, n1)) {
                for (int i = 0; i < n0; i++) {
                    int cellIndex = treeIds[ME][0][i];
                    if (!cells[cellIndex].isDormant) {
                        order = "GROW " + to_string(cellIndex);
                        found = 1;
                        break;
                    }
                }
            }
        }

        // if still order not possible
        if (!found) {
            order = "WAIT";
        }

        cout << order << endl;
        // GROW cellIdx | SEED sourceIdx targetIdx | COMPLETE cellIdx | WAIT <message>
    }
    return 0;
}













