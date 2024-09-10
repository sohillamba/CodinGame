/*
    At each character, find best zone by just calculating moves needed to go there + moves needed to update char at their and then take the minimum.
    Total instructions: 5960
*/

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

const int MAX_ZONES = 40;


// Function to calculate the minimum adjustment between two characters in the cyclic alphabet
int minCharAdjustment(int from, int to) {
    int forward = (to - from + 27) % 27;
    int backward = (from - to + 27) % 27;
    return (forward < backward) ? forward : -backward;
}

int minZoneAdjustment(int from, int to) {
    int forward = (to - from + MAX_ZONES) % MAX_ZONES;
    int backward = (from - to + MAX_ZONES) % MAX_ZONES;
    return (forward < backward) ? forward : -backward;
}



class State {
public:
    int zone[MAX_ZONES];
    int pos;

    State(int _pos = 0) {
        for (int i = 0; i < MAX_ZONES; i++) {
            zone[i] = 26;
        }
        pos = _pos;
    }

    int findBestZone(int ch) {
        int best = pos, moves = abs(minCharAdjustment(zone[pos], ch));
        for (int i = 0; i < MAX_ZONES; i++) {
            int tempMoves = abs(minZoneAdjustment(pos, i)) + abs(minCharAdjustment(zone[i], ch));
            if (tempMoves < moves) {
                moves = tempMoves;
                best = i;
            }
        }
        return best;
    }

    string moveTo(int goal) {
        int moves = minZoneAdjustment(pos, goal);
        pos = goal; // update pos
        return string(abs(moves), moves >= 0 ? '>' : '<');
    }

    string changeTo(int goal) {
        int moves = minCharAdjustment(zone[pos], goal);
        zone[pos] = goal; // update pos zone
        return string(abs(moves), moves >= 0 ? '+' : '-');
    }

    string printChar() {
        return ".";
    }
};


// Function to generate the Brainfuck code to print a given string
string generateBrainfuckCode(const string& input) {
    string bfCode;

    State state;

    for (int i = 0; i < input.size(); ++i) {
        char targetChar = input[i];

        int targetValue = targetChar == ' ' ? 26 : targetChar - 'A';

        int bestZone = state.findBestZone(targetValue);
        bfCode += state.moveTo(bestZone);
        bfCode += state.changeTo(targetValue);
        bfCode += state.printChar();
    }

    return bfCode;
}

int main() {
    string input;
    getline(cin, input);

    string bfCode = generateBrainfuckCode(input);
    cout << bfCode << endl;
    return 0;
}
