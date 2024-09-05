/*
  Thanks to: https://github.com/WildSmilodon/codingame-puzzle-solutions/blob/master/bots/xmas-rush/xmas-rush.cpp
*/

#pragma GCC optimize("-O3","-ffast-math")
#pragma GCC optimize("inline")
#pragma GCC optimize("omit-frame-pointer")
#pragma GCC optimize("unroll-loops")

#include <iostream>
#include <string>
#include <vector>

using namespace std;


const int N = 7;
const int dirRow[4] = {-1, 0, 1, 0};
const int dirCol[4] = {0, 1, 0, -1};
const string dirName[4] = {"UP", "RIGHT", "DOWN", "LEFT"};
const int PUSH_TURN = 0;
const int MOVE_TURN = 1;
const int MY_ID = 0;
const int HIS_ID = 1;
const int IN_MY_HAND = -1;
const int IN_HIS_HAND = -2;
const int MAX_MOVES = 21;
const int UP = 0;
const int RIGHT = 1;
const int DOWN = 2;
const int LEFT = 3;

string MyItemNames[12];
int MyItemPos[12][2];
int MyItemCount;
int MyQuestCount;

string HisItemNames[12];
int HisItemPos[12][2];
int HisItemCount;
int HisQuestCount;

int queueList[1000][2];


int getRandomNumber(int n) {
  return rand() % n;
}

int stringToBinary(string s) {
  int num = 0;
  for (int i = 0; i < 4; i++) {
    if (s[i] == '1') {
      num += 1 << (3 - i);
    }
  }
  return num;
}

bool canGo(int tile, int dir) {
  return (tile >> (3 - dir)) & 1;
}
bool canGoUp(int tile) {
  return (tile & 8) >> 3;
}
bool canGoRight(int tile) {
  return (tile & 4) >> 2;
}
bool canGoDown(int tile) {
  return (tile & 2) >> 1;
}
bool canGoLeft(int tile) {
  return (tile & 1);
}

int initPlayer(int cardsCount, int row, int col, int tile, int i) {
  int p = tile;
  p |= i << 7;            // player id
  p |= row << 8;          // row
  p |= col << 11;         // col
  p |= cardsCount << 14;  // cardsCount
  p |= MAX_MOVES << 18;   // steps left
  return p;
}

// player getters
// int getPlayerTile(int p) {
//   return p & 15;
// }

// bool hasTileOnPlayer(int p) {
//   return (p >> 4) & 1;
// }

// bool hasQuestOnPlayer(int p) {
//   return (p >> 5) & 1;
// }

// bool getPlayerItemOwnerId(int p) {
//   return (p >> 6) & 1;
// }

int getPlayerId(int p) {
  return ((p & 128) >> 7);
}

int getPlayerRow(int p) {
  return (p & 1792) >> 8;
}

int getPlayerCol(int p) {
  return (p & 14336) >> 11;
}

int getPlayerCardsCount(int p) {
  return (p & 245760) >> 14;
}

int getPlayerStepsLeft(int p) {
  return (p & 8126464) >> 18;
}

// player setters
void setPlayerTile(int &p, int tile) {
  p &= 8388480; // clear bits for tile info, 8388480 = 11111111111111110000000
  p |= tile;
}

// void setHasItemOnPlayer(int &p, int hasItem) {
//   // p &= ((1 << 30) - 1) ^ (1 << 4); // wrong
//   p |= (hasItem << 4);
// }

// void setHasQuestOnPlayer(int &p, int hasQuest) {
//   // p &= ((1 << 30) - 1) ^ (1 << 5); // wrong
//   p |= (hasQuest << 5);
// }

// void setItemOwnerId(int &p, int id) {
//   // p &= ((1 << 30) - 1) ^ (1 << 6); // wrong
//   p |= (id << 6);
// }

// void setPlayerId(int &p, int id) {
//   p &= ((1 << 30) - 1) ^ (1 << 7); // wrong
//   p |= (id << 7);
// }

void setPlayerRow(int &p, int row) {
  p &= 8386815; // clear bits for row, 8386815 = 11111111111100011111111
  p |= (row << 8);
}

void setPlayerCol(int &p, int col) {
  p &= 8374271; // clear bits for col, 8374271 = 11111111100011111111111
  p |= (col << 11);
}

// void setPlayerCardsCount(int &p, int cardsCount) {
//   // p &= ((1 << 30) - 1) ^ (1 << 14); // wrong
//   p |= (cardsCount << 14);
// }

void setPlayerStepsLeft(int &p, int stepsLeft) {
  p &= 262143; // clear bits for steps left, 262143 = 0000111111111111111111
  p |= (stepsLeft << 18);
}



// grid setters and getters

void setGridTile(int &cell, int tile) {
  setPlayerTile(cell, tile); // same functionality
}

void setGridDist(int &cell, int val) { // Set distance in grid
  cell &= 8384639; // clear bits for dist, 8384639 =11111111111000001111111
  cell |= (val << 7); // write value to bits 7-11
}

int getGridDist(int cell) {  // get BFS distance from drig
  return (cell & 3968) >> 7;
}


int getTile (int cell) {
  return (cell) & 15;
}

int hasItem(int cell) {
  return ((cell & 16) >> 4); 
}

int hasQuest(int cell) {
  return ((cell & 32) >> 5);
}

int itemOwner(int cell) {
  return ((cell & 64) >> 6);
}


int hasPlayersQuest(int cell, int p) {
    return ( hasQuest(cell) & (~(getPlayerId(p) ^ itemOwner(cell))) );  // not (xor)
}

int hasPlayersItem(int cell, int p) {
    return ( hasItem(cell) & (~(getPlayerId(p) ^ itemOwner(cell))) );
}


void clearItem(int &cell) {
  cell &= ~(1u << 4); // AND with ...1111101111 -> clearing 4th bit
}

void clearQuest(int &cell) {
  cell &= ~(1u << 5);
}


class Game {
public:
  int player[2];  // bit    0-3   = my tile directions (0=UP, 1=RIGHT, 2=DOWN, 3=LEFT)
                  // bit      4   = item on my tile (1=yes, 2=no)
                  // bit      5   = quest on my tile (1=yes, 2=no)
                  // bit      6   = item/quest owner (1=ME, 2=HE)
                  // bit      7   = player id (1=HE, 0=ME)
                  // bit   8-10   = row
                  // bit  11-13   = col
                  // bit  14-17   = number of cards left
                  // bit  18-22   = number of steps left

  int grid[N][N]; // bit    0-3   = tile direction (0=UP, 1=RIGHT, 2=DOWN, 3=LEFT)
                  // bit      4   = item on tile (1=yes, 0=no)
                  // bit      5   = quest on tile (1=yes, 0=no)
                  // bit      6   = item/quest owner (1=HE, 0=ME)
                  // bit   7-11   = dist, BFS result

  int turnType;

  int score (int pId) {
    int s = 0;
    BFS(getPlayerRow(player[pId]), getPlayerCol(player[pId]));
    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
        if (getGridDist(grid[i][j]) < getPlayerStepsLeft(player[pId])) {
          s++;
          if (hasPlayersQuest(grid[i][j], player[pId])) {
            s += 10000 - 100 * getGridDist(grid[i][j]);
          }
          if (hasPlayersItem(grid[i][j], player[pId])) {
            s += 10;
          }
        }
      }
    }
    return s;
  }

  void BFS (int row, int col) { // depth first search
    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
        setGridDist(grid[i][j], 31); // set large distance, 31 is max distance we can store as we only have 5 bits for distance
      }
    }

    int start = 0, end = 0; // start and end pointers for the queue list to do dfs
    int cRow = row;
    int cCol = col;
    int cDist = 0;

    queueList[end][0] = cRow;
    queueList[end][1] = cCol;
    end++;

    setGridDist(grid[cRow][cCol], cDist);
    while (start < end) {
      cRow = queueList[start][0];
      cCol = queueList[start][1];
      start++; // queue pop
      cDist = getGridDist(grid[cRow][cCol]) + 1;
      
      if (cRow > 0) {
        if (cDist < getGridDist(grid[cRow-1][cCol]) && canGoUp(grid[cRow][cCol]) && canGoDown(grid[cRow-1][cCol])) {
          queueList[end][0] = cRow - 1;
          queueList[end][1] = cCol;
          end++;
          setGridDist(grid[cRow-1][cCol], cDist);
        }
      }
      if (cRow < N - 1) {
        if (cDist < getGridDist(grid[cRow+1][cCol]) && canGoDown(grid[cRow][cCol]) && canGoUp(grid[cRow+1][cCol])) {
          queueList[end][0] = cRow + 1;
          queueList[end][1] = cCol;
          end++;
          setGridDist(grid[cRow+1][cCol], cDist);
        }
      }
      if (cCol > 0) {
        if (cDist < getGridDist(grid[cRow][cCol-1]) && canGoLeft(grid[cRow][cCol]) && canGoRight(grid[cRow][cCol-1])) {
          queueList[end][0] = cRow;
          queueList[end][1] = cCol - 1;
          end++;
          setGridDist(grid[cRow][cCol-1], cDist);
        }
      }
      if (cCol < N - 1) {
        if (cDist < getGridDist(grid[cRow][cCol+1]) && canGoRight(grid[cRow][cCol]) && canGoLeft(grid[cRow][cCol+1])) {
          queueList[end][0] = cRow;
          queueList[end][1] = cCol + 1;
          end++;
          setGridDist(grid[cRow][cCol+1], cDist);
        }
      }
    }
  }

  string goTo (int pId, int tRow, int tCol) {
    BFS(getPlayerRow(player[pId]), getPlayerCol(player[pId]));

    int cRow = tRow;
    int cCol = tCol;
    int cDist = getGridDist(grid[cRow][cCol]);
    int d = cDist;

    string orders[25];
    int nOrders = 0;
    for (int i = 0; i < d; i++) {
      ///////////////////////////////////////////
      // todo: make sure below algo is correct //
      ///////////////////////////////////////////
      // same algo also used in pick items function
      
      if (cRow > 0) {
        if (getGridDist(grid[cRow-1][cCol]) == cDist - 1 && canGoUp(grid[cRow][cCol]) && canGoDown(grid[cRow-1][cCol])) { 
          orders[nOrders] = "DOWN"; cRow--;
        } 
      }
      if (cRow < N - 1) {
        if (getGridDist(grid[cRow+1][cCol]) == cDist - 1 && canGoDown(grid[cRow][cCol]) && canGoUp(grid[cRow+1][cCol])) { 
          orders[nOrders] = "UP"; cRow++;
        } 
      }
      if (cCol > 0) { 
        if (getGridDist(grid[cRow][cCol-1]) == cDist - 1 && canGoLeft(grid[cRow][cCol]) && canGoRight(grid[cRow][cCol-1])) { 
          orders[nOrders] = "RIGHT"; cCol--;
        } 
      }
      if (cCol < N - 1) { 
        if (getGridDist(grid[cRow][cCol+1]) == cDist - 1 && canGoRight(grid[cRow][cCol]) && canGoLeft(grid[cRow][cCol+1])) { 
          orders[nOrders] = "LEFT"; cCol++;
        } 
      }
      nOrders++;
      cDist--;
    }

    string moves = "";
    for (int k = nOrders - 1; k >= 0; k--) {
      moves += " " + orders[k];
    }

    int stepsLeft = getPlayerStepsLeft(player[pId]);
    // update steps left
    setPlayerStepsLeft(player[pId], stepsLeft - d);

    return moves;
  }

  string pickUpItems (int pId) {
    int stepsLeft = getPlayerStepsLeft(player[pId]);
    BFS(getPlayerRow(player[pId]), getPlayerCol(player[pId]));

    string moves = "";

    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
        if (hasPlayersQuest(grid[i][j], player[pId])) {
          int d = getGridDist(grid[i][j]);
          if (d < stepsLeft) {
            // quest is reachable
            // cerr << "q " << i << " " << j << " " << d << endl;
            int cRow = i, cCol = j, cDist = d;

            string orders[25];
            int nOrders = 0;
            for (int k = 0; k < d; k++) {
              int step;
              ///////////////////////////////////////////
              // todo: make sure below algo is correct //
              ///////////////////////////////////////////
              if (cRow > 0) {
                if (getGridDist(grid[cRow-1][cCol]) == cDist - 1 && canGoUp(grid[cRow][cCol]) && canGoDown(grid[cRow-1][cCol])) { 
                  orders[nOrders] = "DOWN"; cRow--;
                } 
              }
              if (cRow < N - 1) {
                if (getGridDist(grid[cRow+1][cCol]) == cDist - 1 && canGoDown(grid[cRow][cCol]) && canGoUp(grid[cRow+1][cCol])) { 
                  orders[nOrders] = "UP"; cRow++;
                } 
              }
              if (cCol > 0) { 
                if (getGridDist(grid[cRow][cCol-1]) == cDist - 1 && canGoLeft(grid[cRow][cCol]) && canGoRight(grid[cRow][cCol-1])) { 
                  orders[nOrders] = "RIGHT"; cCol--;
                } 
              }
              if (cCol < N - 1) { 
                if (getGridDist(grid[cRow][cCol+1]) == cDist - 1 && canGoRight(grid[cRow][cCol]) && canGoLeft(grid[cRow][cCol+1])) { 
                  orders[nOrders] = "LEFT"; cCol++;
                } 
              }
              nOrders++;
              cDist--;
            }

            for (int k = nOrders - 1; k >= 0; k--) {
              moves += " " + orders[k];
            }

            // update steps left
            stepsLeft -= d;
            setPlayerStepsLeft(player[pId], stepsLeft);

            // I am now at quest item position
            setPlayerRow(player[pId], i);
            setPlayerCol(player[pId], j);

            // Make quest item dissapear
            clearItem(grid[i][j]);
            clearQuest(grid[i][j]);

            // Found one item, is there another?
            BFS(getPlayerRow(player[pId]), getPlayerCol(player[pId]));
          }
        }
      }
    }

    return moves;
  }

  void push (int rc, int dir, int pId) { // push one row or column in dir
    if (dir == UP) { // push rc column up
      int cell = grid[0][rc]; // cell to wrap around
      for (int i = 0; i < 6; i++) {
        grid[i][rc] = grid[i+1][rc]; // move each cell
      }
      // copy hand tile to grid
      setGridTile(grid[6][rc], player[pId] & 127); // copy only 7 bits
      // copy grid tile to hand
      setPlayerTile(player[pId], cell & 127); // copy only 7 bits
      // teleport player
      for (int k = 0; k < 2; k++) {
        if (getPlayerCol(player[k]) == rc) {
          int row = getPlayerRow(player[k]) - 1;
          if (row < 0) row = 6;
          setPlayerRow(player[k], row);
        }
      }
    }

    if (dir == RIGHT) { // push rc row right
      int cell = grid[rc][6]; // cell to wrap around
      for (int i = 6; i > 0; i--) {
        grid[rc][i] = grid[rc][i-1]; // move each cell
      }
      // copy hand tile to grid
      setGridTile(grid[rc][0], player[pId] & 127); // copy only 7 bits
      // copy grid tile to hand
      setPlayerTile(player[pId], cell & 127); // copy only 7 bits
      // teleport player
      for (int k = 0; k < 2; k++) {
        if (getPlayerRow(player[k]) == rc) {
          int col = getPlayerCol(player[k]) + 1;
          if (col > 6) col = 0;
          setPlayerCol(player[k], col);
        }
      }
    }

    if (dir == DOWN) {
      int cell = grid[6][rc]; // cell to wrap around
      for (int i = 6; i > 0; i--) {
        grid[i][rc] = grid[i-1][rc]; // move each cell
      }
      // copy hand tile to grid
      setGridTile(grid[0][rc], player[pId] & 127); // copy only 7 bits
      // copy grid tile to hand
      setPlayerTile(player[pId], cell & 127); // copy only 7 bits
      // teleport player
      for (int k = 0; k < 2; k++) {
        if (getPlayerCol(player[k]) == rc) {
          int row = getPlayerRow(player[k]) + 1;
          if (row > 6) row = 0;
          setPlayerRow(player[k], row);
        }
      }
    }

    if (dir == LEFT) {
      int cell = grid[rc][0]; // cell to wrap around
      for (int i = 0; i < 6; i++) {
        grid[rc][i] = grid[rc][i+1]; // move each cell
      }
      // copy hand tile to grid
      setGridTile(grid[rc][6], player[pId] & 127); // copy only 7 bits
      // copy grid tile to hand
      setPlayerTile(player[pId], cell & 127); // copy only 7 bits
      // teleport player
      for (int k = 0; k < 2; k++) {
        if (getPlayerRow(player[k]) == rc) {
          int col = getPlayerCol(player[k]) - 1;
          if (col < 0) col = 6;
          setPlayerCol(player[k], col);
        }
      }
    }
  }

  void read () {
    cin >> turnType; cin.ignore();

    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
        string tile;
        cin >> tile; cin.ignore();
        // setGridTile(grid[i][j], stringToBinary(tile)); // wrong as initially grid[i][j] can have garbage values and setGridTile only changes tile bites of it other bites will be same
        grid[i][j] = stringToBinary(tile);
      }
    }

    for (int i = 0; i < 2; i++) {
      int cardsCount, row, col;
      string tile;
      cin >> cardsCount >> col >> row >> tile; cin.ignore();
      player[i] = initPlayer(cardsCount, row, col, stringToBinary(tile), i);
    }

    int numItems;
    cin >> numItems; cin.ignore();
    MyItemCount = 0;
    HisItemCount = 0;
    for (int i = 0; i < numItems; i++) {
      string name;
      int row, col, itemPlayerId;
      cin >> name >> col >> row >> itemPlayerId; cin.ignore();

      if (itemPlayerId == MY_ID) {
        MyItemNames[MyItemCount] = name;
        MyItemPos[MyItemCount][0] = row;
        MyItemPos[MyItemCount][1] = col;
        MyItemCount++;
      } else {
        HisItemNames[HisItemCount] = name;
        HisItemPos[HisItemCount][0] = row;
        HisItemPos[HisItemCount][1] = col;
        HisItemCount++;
      }
      if (row < 0) {
        player[itemPlayerId] += 1 << 4; // has item
        // player[itemPlayerId] += (row == IN_MY_HAND ? 0 : 1) << 6; // item owner (feels right, check it late)
        player[itemPlayerId] += (itemPlayerId << 6); // item owner // maybe wrong but working
      } else {
        grid[row][col] += 1 << 4; // has item
        grid[row][col] += (itemPlayerId) << 6; // item owner
      }
    }

    int numQuests;
    cin >> numQuests; cin.ignore();
    MyQuestCount = 0;
    HisQuestCount = 0;
    for (int i = 0; i < numQuests; i++) {
      string itemName;
      int questPlayerId;
      cin >> itemName >> questPlayerId; cin.ignore();

      if (questPlayerId == MY_ID) {
        MyQuestCount++;
        for (int j = 0; j < MyItemCount; j++) {
          if (MyItemNames[j] == itemName) {
            int row = MyItemPos[j][0];
            int col = MyItemPos[j][1];
            if (row < 0) {
              // working logic is:
              // in both below cases:
              player[questPlayerId] += (1 << 5);
              // if (row == IN_MY_HAND) {
              //   player[MY_ID] += 1 << 5;      // has quest
              //   player[MY_ID] += MY_ID << 6;  // quest owner, already done
              // } else {
              //   player[HIS_ID] += 1 << 5;     // has quest
              //   player[HIS_ID] += MY_ID << 6; // quest owner, already done
              // }
            } else {
              grid[row][col] += 1 << 5;       // has quest
              // grid[row][col] += MY_ID << 6;   // quest owner, already done
            }
            // break;
          }
        }
      } else {
        HisQuestCount++;
        for (int j = 0; j < HisItemCount; j++) {
          if (HisItemNames[j] == itemName) {
            int row = HisItemPos[j][0];
            int col = HisItemPos[j][1];
            if (row < 0) {
              // working logic is:
              // in both below cases:
              player[questPlayerId] += (1 << 5);
              // if (row == IN_MY_HAND) {
              //   player[MY_ID] += 1 << 5;       // has quest
              //   player[MY_ID] += HIS_ID << 6;  // quest owner
              // } else {
              //   player[HIS_ID] += 1 << 5;      // has quest
              //   player[HIS_ID] += HIS_ID << 6; // quest owner
              // }
            } else {
              grid[row][col] += 1 << 5;        // has quest
              // grid[row][col] += HIS_ID << 6;   // quest owner
            }
            // break;
          }
        }
      }
    }
  }


};


bool isBoardChanged (Game &g1, Game &g2) {
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      if (getTile(g1.grid[i][j]) != getTile(g2.grid[i][j])) {
        return 1;
      }
    }
  }
  return 0;
}
bool isSameRowColPush(int myRc, int myD, int hisRc, int hisD) { // is both players pushing same row/col
  if (myRc != hisRc) return 0;
  if ((myD == 0 || myD == 2) && (hisD == 1 || hisD == 3)) return 0;
  if ((myD == 1 || myD == 3) && (hisD == 0 || hisD == 2)) return 0;
  return 1;
}

int otherPId (int pId) {
  if (pId == MY_ID) return HIS_ID;
  else return MY_ID;
}

void pushOnce(Game game, int pId, int &Trc, int &Td, int &maxS) {
  maxS = -10000;
  for (int rc = 0; rc < N; rc++) {
    for (int d = 0; d < 4; d++) {
      Game g = game;
      g.push(rc, d, pId);
      int score = g.score(pId);
      // cerr << rc << " " << d << " s= " << score << endl;
      if (score > maxS) {
        maxS = score;
        Trc = rc;
        Td = d;
      }
    }
  }
}

void pushOnceOnce(Game game, int pId, int &Trc, int &Td, int &maxS) {
  maxS = -10000;
  for (int rc = 0; rc < N; rc++) {
    for (int d = 0; d < 4; d++) {
      Game g = game;
      g.push(rc, d, pId);
      int score = g.score(pId);
      int Frc, Fd, Fscore;
      pushOnce(g, pId, Frc, Fd, Fscore);
      score = 10 * score + Fscore;
      // cerr << rc << " " << d << " s= " << score << " " << Fscore << endl;
      if (score > maxS) {
        maxS = score;
        Trc = rc;
        Td = d;
      }
    }
  }
}

void pushTwice(Game g, int pId, int &TargetRC, int &TargetDir, int &maxScore) {
  int Trc = 0, Td = 0, maxS = -1;
  Game game = g;

  pushOnce(game, otherPId(pId), Trc, Td, maxS); // find what will he do
  game.push(Trc, Td, otherPId(pId)); // Make his move
  
  pushOnce(game, pId, Trc, Td, maxS); // Now consider my moves

  // Remember results
  TargetRC = Trc;
  TargetDir = Td;
  maxScore = maxS;
}

void pushTwiceOnce(Game g, int pId, int &TargetRC, int &TargetDir, int &maxScore) {
  int Trc = 0, Td = 0, maxS = -1;
  Game game = g;

  pushOnce(game, otherPId(pId), Trc, Td, maxS); // find what will he do
  game.push(Trc, Td, otherPId(pId)); // Make his move
  
  pushOnceOnce(game, pId, Trc, Td, maxS); // Now consider my moves

  // Remember results
  TargetRC = Trc;
  TargetDir = Td;
  maxScore = maxS;
}

void pushBoth(Game g, int pId, int &TargetRC, int &TargetDir, int &maxScore) {
  int Trc = 0, Td = 0, maxS = -1000000;

  for (int myRc = 0; myRc < 7; myRc++) {
    for (int myD = 0; myD < 4; myD++) {
      int hisMinScore = 10000000;
      for (int hisRc = 0; hisRc < 7; hisRc++) {
        for (int hisD = 0; hisD < 4; hisD++) {
          Game game = g;
          if (!isSameRowColPush(myRc, myD, hisRc, hisD)) {
            game.push(myRc, myD, pId);
            game.push(hisRc, hisD, otherPId(pId));
          }
          int score = game.score(pId) - game.score(otherPId(pId));
          if (score < hisMinScore) {
            hisMinScore = score;
          }
        }
      }
      if (hisMinScore > maxS) {
        maxS = hisMinScore;
        Trc = myRc;
        Td = myD;
      }
    }
  }
  // Remember results
  TargetRC = Trc;
  TargetDir = Td;
  maxScore = maxS;
}

void whereToMove(Game game, int pId, int &Trow, int &Tcol) {
  int maxS = -10000000, Trc, Td, score;
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      if (getGridDist(game.grid[i][j]) < getPlayerStepsLeft(game.player[pId])) {
        Game g = game;
        setPlayerRow(g.player[pId], i);
        setPlayerCol(g.player[pId], j);
        // pushOnce(g, pId, Trc, Td, score);
        pushBoth(g, pId, Trc, Td, score);
        if (score > maxS) { maxS = score, Trow = i; Tcol = j; }
        // cerr << i << " " << j << " " << score << " " << maxS << " " << Trow << " " << Tcol << " trc " << Trc << " " << Td << endl;
      }
    }
  }
}

string getPushOrder(int rc, int dir) {
  string order = "PUSH ";
  order += to_string(rc);
  order += " " + dirName[dir];
  return order;
}


int main() {
  srand(time(NULL));

  Game game, oldGame;
  int gridLockedCounter = 0;

  // game loop
  while (1) {
    game.read();

    if (game.turnType == PUSH_TURN) {
      if (isBoardChanged(game, oldGame)) {
        gridLockedCounter = 0;
      } else {
        gridLockedCounter++;
      }
      oldGame = game;

      int rc, dir, score = -1;
      
      // deadlock prevention
      if (gridLockedCounter < 5 || getPlayerCardsCount(game.player[MY_ID]) < getPlayerCardsCount(game.player[HIS_ID])) {
        // pushOnce(game, MY_ID, rc, dir, score);
        // pushTwice(game, MY_ID, rc, dir, score);
        pushBoth(game, MY_ID, rc, dir, score);
        // pushTwiceOnce(game, MY_ID, rc, dir, score);
      } else {
        rc = getRandomNumber(N);
        dir = getRandomNumber(4);
      }
      cout << getPushOrder(rc, dir) << endl;

    } else {
      string orders = game.pickUpItems(MY_ID); // pick up items if possible

      int row, col;
      whereToMove(game, MY_ID, row, col); // move to the best plase according to next best push

      orders += game.goTo(MY_ID, row, col);

      if (orders == "") {
        cout << "PASS" << endl;
      } else {
        cout << "MOVE" << orders << endl;
      }
    }
    // break;
  }
  
  return 0;
}

























