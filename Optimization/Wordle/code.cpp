// Total guesses: 211
// Rank: 370

#pragma GCC optimize("-O3","-ffast-math")
#pragma GCC optimize("inline")
#pragma GCC optimize("omit-frame-pointer")
#pragma GCC optimize("unroll-loops")

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
using namespace std;

class Game {
public:
    int N;
    int n;
    vector<string> words;

    vector<int> f;
    vector<vector<int>> f2;
    
    int state[6];
    int guessIdx = 0;
    int guessIdx1 = 0, guessIdx2 = 1;
    vector<int> cur;

    vector<int> score, uniqueCharCount;
    vector<int> vis;

    Game() {
        f = vector<int>(26);
        f2 = vector<vector<int>>(6, vector<int>(26));
    }
    void init() {
        cin >> n;
        N = n;
        words.resize(n);
        cur.resize(n);
        for (int i = 0; i < n; i++) {
            cin >> words[i]; cin.ignore();
            cur[i] = i;
        }
        update();
        findFirstAndSecondGuess();
        // vis.resize(n, 0);
    }

    void update() {
        updateFreq();
        updateScore();
        updateUniqueCharCount();
    }

    void readState() {
        for (int i = 0; i < 6; i++) {
            cin >> state[i]; cin.ignore();
        }
    }

    void updateFreq() {
        f = vector<int>(26);
        f2 = vector<vector<int>>(6, vector<int>(26));
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < 6; j++) {
                f[words[cur[i]][j]-'A']++;
                f2[j][words[cur[i]][j]-'A']++;
            }
        }
    }

    void updateScore() {
        score.resize(n, 0);
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < 6; j++) {
                score[i] += f[words[cur[i]][j]-'A'];
                score[i] += f2[j][words[cur[i]][j]-'A'];
            }
        }
    }

    void updateUniqueCharCount() {
        uniqueCharCount.resize(n, 0);
        for (int i = 0; i < n; i++) {
            for (char &ch : words[cur[i]]) {
                uniqueCharCount[i] |= 1 << (ch - 'A');
            }
        }
    }

    void findFirstAndSecondGuess() {
        guessIdx1 = 0;
        guessIdx2 = 1;
        // return;

        int mx = score[0] + score[1], chCount = 0;
        for (int i = 0; i < n; i++) {
            for (int j = i + 1; j < n; j++) {
                int x = score[i] + score[j];
                int y = __builtin_popcount(uniqueCharCount[i] | uniqueCharCount[j]);
                if (y > chCount) {
                    chCount = y;
                    mx = x;
                    guessIdx1 = i;
                    guessIdx2 = j;
                } else if (y == chCount && x > mx) {
                    guessIdx1 = i;
                    guessIdx2 = j;
                    mx = x;
                }
            }
        }

        // cerr << words[guessIdx1] << " " << words[guessIdx2] << "\n";
        // cerr << uniqueCharCount[guessIdx1] << " " << uniqueCharCount[guessIdx2] << "\n";
        // for (int i = 0; i < 26; i++) {
        //     cerr << ((uniqueCharCount[guessIdx1] & (1 << i)) ? 1 : 0);
        // }
        // cerr << "\n";
        // for (int i = 0; i < 26; i++) {
        //     cerr << ((uniqueCharCount[guessIdx2] & (1 << i)) ? 1 : 0);
        // }
    }

    void play(int turn) {
        if (turn == 0) {
            guessIdx = guessIdx1;
        } else if (turn == 1) {
            guessIdx = guessIdx2;
        } else {
            int newGuessIdx = 0;
            // while (vis[cur[newGuessIdx]]) newGuessIdx++;
            for (int i = 0; i < n; i++) {
                // if (vis[cur[i]]) continue;
                int x = __builtin_popcount(uniqueCharCount[cur[i]]);
                int y = __builtin_popcount(uniqueCharCount[cur[newGuessIdx]]);
                if (x > y) {
                    newGuessIdx = i;
                } else if (x == y && score[cur[i]] > score[cur[newGuessIdx]]) {
                    newGuessIdx = i;
                }
            }
            guessIdx = newGuessIdx;
        }

        cout << words[cur[guessIdx]] << endl;
        // vis[cur[guessIdx]] = 1;

        readState();

        vector<int> discard(n);
        for (int i = 0; i < 6; i++) {
            if (state[i] == 1) { // red
                for (int j = 0; j < n; j++) {
                    for (int k = 0; k < 6; k++) {
                        if (words[cur[j]][k] == words[cur[guessIdx]][i]) {
                            discard[j] = 1;
                            break;
                        }
                    }
                }
            } else if (state[i] == 2) { // yellow
                for (int j = 0; j < n; j++) {
                    bool has = 0;
                    for (int k = 0; k < 6; k++) {
                        if (words[cur[j]][k] == words[cur[guessIdx]][i]) {
                            has = 1;
                            break;
                        }
                    }
                    if (!has) {
                        discard[j] = 1;
                    }
                }
            } else { // greeen
                for (int j = 0; j < n; j++) {
                    if (words[cur[j]][i] != words[cur[guessIdx]][i]) {
                        discard[j] = 1;
                    }
                }
            }
        }
        discard[guessIdx] = 1; // discard current guess

        vector<int> cur2;
        for (int i = 0; i < n; i++) {
            if (!discard[i]) {
                cur2.push_back(cur[i]);
            }
        }

        cur = cur2;
        n = cur.size();
        // update();
    }
};

int main() {
    ios_base::sync_with_stdio(false);cin.tie(NULL);cout.tie(NULL);
    srand(time(NULL));

    Game game;
    game.init();
    game.readState();

    for (int turn = 0; ; turn++) {
        game.play(turn);
    }
}









