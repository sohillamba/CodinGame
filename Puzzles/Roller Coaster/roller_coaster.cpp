#include <iostream>
#include <vector>

using namespace std;

long long solve() {
    int seats, times, n; // n = groups
    cin >> seats >> times >> n; cin.ignore();

    vector<int> groups(n);
    int totalSum = 0;
    for (int i = 0; i < n; i++) {
        cin >> groups[i]; cin.ignore();
        totalSum += groups[i];
    }

    if (totalSum <= seats) {
        return 1ll * totalSum * times;
    }


    vector<int> nextIdx(n), count(n);
    for (int i = 0; i < n; i++) {
        for (int j = i; count[i] + groups[j] <= seats; j = (j + 1) % n) {
            count[i] += groups[j];
            nextIdx[i] = j;
        }
        nextIdx[i] = (nextIdx[i] + 1) % n;
    }


    long long ans = 0;
    for (int t = 0, i = 0; t < times; t++) {
        ans += count[i];
        i = nextIdx[i];
    }

    return ans;
}


int main() {
    cout << solve() << endl;
}





