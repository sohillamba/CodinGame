/*
	Just using only one zone to print all characters
	Total instuctions: 11489
*/

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

// Function to calculate the minimum adjustment between two characters in the cyclic alphabet
int minAdjustment(int from, int to) {
    int forward = (to - from + 27) % 27;
    int backward = (from - to + 27) % 27;
    return (forward < backward) ? forward : -backward;
}

// Function to generate the Brainfuck code to print a given string
string generateBrainfuckCode(const string& input) {
    string bfCode;
    int pointerPosition = 0;
    int currentChar = 26;

    for (int i = 0; i < input.size(); ++i) {
        char targetChar = input[i];

        int targetValue;
        if (targetChar == ' ') {
            targetValue = 26;
        } else {
            targetValue = targetChar - 'A';
        }

        int adjustment = minAdjustment(currentChar, targetValue);

        if (adjustment > 0) {
            bfCode += string(adjustment, '+');
        } else if (adjustment < 0) {
            bfCode += string(-adjustment, '-');
        }

        bfCode += ".";

        currentChar = targetValue;
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
