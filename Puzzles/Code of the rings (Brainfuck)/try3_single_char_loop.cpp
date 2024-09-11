/*
    Implemented single character loops (only read loops)
*/

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

const int MAX_ZONES = 30;

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
            zone[i] = 26;  // Initialize all zones to 'space' (index 26)
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
        pos = goal;  // Update position
        return string(abs(moves), moves >= 0 ? '>' : '<');
    }

    string changeTo(int goal) {
        int moves = minCharAdjustment(zone[pos], goal);
        zone[pos] = goal;  // Update the character in the current zone
        return string(abs(moves), moves >= 0 ? '+' : '-');
    }

    string printChar() {
        return ".";
    }

    // Efficiently handle repeated characters using a read loop
    string handleReadLoop(int ch, int repeatCount) {
        string code;
        code += changeTo(ch);                 // Set the first character (e.g., 'S')
        code += moveTo((pos+1) % MAX_ZONES);  // Move to the next memory zone for the counter
        code += changeTo(repeatCount - 1);
        code += "[<.>-]";                     // Loop to print the character multiple times

        // Update the zones and pos after the loop
        zone[pos] = 26;                       // The counter will be zero after the loop
        // code += "<"; pos--;                   // Move back to the previous position after the loop ends

        return code;
    }
};

string generateBrainfuckCode(const vector<int>& input, int maxRepeatCountAllowd = 1) {
    string bfCode;
    State state;

    for (int i = 0; i < input.size(); ++i) {
        int repeatCount = 1;

        // Find how many consecutive characters are the same
        while (i + 1 < input.size() && input[i] == input[i + 1] && repeatCount < 26) {
            repeatCount++;
            i++;
        }

        // Find the best zone and apply the read loop if there are repetitions
        int bestZone = state.findBestZone(input[i]);
        bfCode += state.moveTo(bestZone);

        if (repeatCount > maxRepeatCountAllowd) {
            bfCode += state.handleReadLoop(input[i], repeatCount);
        } else {
            for (int j = 0; j < repeatCount; j++) {
                bfCode += state.changeTo(input[i]); // Change to the character if it's not repeated
                bfCode += state.printChar();        // Print the character
            }
        }
    }

    return bfCode;
}

string compressCode(const string &code) {
    string ans = "";
    char ch = code[0];
    int count = 1;
    for (int i = 1; i < code.size(); i++) {
        if (code[i] == ch) {
            count++;
        } else {
            if ((ch == '+' || ch == '-') && count > 13) {
                ch = (ch == '+') ? '-' : '+';
                count = (27 - count);
            } else if ((ch == '>' || ch == '<') && count > 15) {
                ch = (ch == '>') ? '<' : '>';
                count = (30 - count);
            }
            ans += string(count, ch);
            ch = code[i];
            count = 1;
        }
    }
    if (count > 0) {
        if ((ch == '+' || ch == '-') && count > 13) {
            ch = (ch == '+') ? '-' : '+';
            count = (27 - count);
        } else if ((ch == '>' || ch == '<') && count > 15) {
            ch = (ch == '>') ? '<' : '>';
            count = (30 - count);
        }
        ans += string(count, ch);
    }
    while (ans.size() > 0 && (ans.back() == '<' || ans.back() == '>' || ans.back() == '+' || ans.back() == '-')) ans.pop_back();
    reverse(ans.begin(), ans.end());
    while (ans.size() > 0 && (ans.back() == '<' || ans.back() == '>')) ans.pop_back();
    reverse(ans.begin(), ans.end());
    return ans;
}

string solve(const vector<int> &input) {
    string ans = generateBrainfuckCode(input, 1);
    for (int i = 2; i <= 26; i++) {
        string temp = generateBrainfuckCode(input, i);
        temp = compressCode(temp);
        if (temp.size() < ans.size()) {
            ans = temp;
        }
    }
    return ans;
}

int main() {
    string inputString;
    getline(cin, inputString);

    vector<int> input;
    for (char &ch : inputString) {
        input.push_back(ch == ' ' ? 26 : ch - 'A');  // Map 'A' to 'Z' to indices 0 to 25, and space to 26
    }

    // string bfCode = generateBrainfuckCode(input);
    string bfCode = solve(input);


    cout << bfCode << endl;
    return 0;
}
