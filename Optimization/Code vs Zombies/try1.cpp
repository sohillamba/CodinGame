#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <climits>
#include <random>

using namespace std;

const int H = 9000;
const int W = 16000;
const double PI = 3.14159265358979323846;
const double ep = 1e-9;

std::mt19937 gen(static_cast<unsigned int>(time(NULL)));
int randomInt(int a, int b) {
    std::uniform_int_distribution<> disInt(a, b);
    return disInt(gen);
}
float randomFloat(float a, float b) {
    std::uniform_real_distribution<> disFloat(a, b);
    return disFloat(gen);
}


class Point {
public:
    int x, y;

    Point() = default;
    Point(const int& xx, const int& yy) : x(xx), y(yy) {}

    inline Point operator+(const Point& b) const { return Point(x + b.x, y + b.y); }
    inline Point operator-(const Point& b) const { return Point(x - b.x, y - b.y); }

    friend int distance2(const Point a, const Point b) {
        return (a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y);
    }
    friend double distance(const Point a, const Point b) {
        return sqrtl(double(distance2(a, b)));
    }

    friend Point polarToCart(const float &angle, const int &radius) {
        return Point(radius * cos(angle), radius * sin(angle));
    }
    friend Point polarToCart(const Point &p) {
        return Point(p.y * cos(p.x), p.y * sin(p.x));
    }

    void clamp() {
        if (x < 0) x = 0;
        if (x >= W) x = W - 1;
        if (y < 0) y = 0;
        if (y >= H) y = W - 1;
    }
};

class State {
public:
    int humanCount, zombieCount;
    Point player;
    vector<Point> humans, zombies, zombiesNext;

    State() = default;

    void clear() {
        humans.clear();
        zombies.clear();
        zombiesNext.clear();
    }

    void read() {
        clear();
        cin >> player.x >> player.y; cin.ignore();

        cin >> humanCount; cin.ignore();
        humans.resize(humanCount);
        for (int i = 0, id; i < humanCount; i++) {
            cin >> id >> humans[i].x >> humans[i].y; cin.ignore();
        }

        cin >> zombieCount; cin.ignore();
        zombies.resize(zombieCount);
        zombiesNext.resize(zombieCount);
        for (int i = 0, id; i < zombieCount; i++) {
            cin >> id >> zombies[i].x >> zombies[i].y >> zombiesNext[i].x >> zombiesNext[i].y; cin.ignore();
        }
    }

    string getCommand() {
        int target[zombieCount];
        for (int i = 0; i < zombieCount; i++) {
            int minDis2 = distance2(zombies[i], player);
            target[i] = -1;
            for (int j = 0; j < humanCount; j++) {
                int dist2 = distance2(zombies[i], humans[j]);
                if (dist2 < minDis2) {
                    minDis2 = dist2;
                    target[i] = j;
                }
            }
        }

        // Point randomPoint = Point(randomFloat(0, PI), randomFloat(0, 1000));
        // Point playerTarget = player + polarToCart(randomPoint);
        // playerTarget.clamp();
        Point playerTarget = player;
        int minDis2 = -1;

        for (int h = 0; h < humanCount; h++) {
            int minDisZtoH = INT_MAX, zId = -1;
            for (int z = 0; z < zombieCount; z++) {
                if (target[z] == h) {
                    int temp = distance2(humans[h], zombies[z]);
                    if (temp < minDisZtoH) {
                        minDisZtoH = temp;
                        zId = z;
                    }
                }
            }
            // cerr << minDisZtoH << " " << zId << endl;

            double zombieDis = sqrtl((double)minDisZtoH);
            double playerDis = distance(player, humans[h]);
            int playerTime = ceil(playerDis / 1000.0), zombieTime = ceil(zombieDis / 400.0);

            cerr << zombieDis << " " << playerDis << ", ";
            cerr << zombieTime << " " << playerTime << endl;

            if (zId != -1) {
                if (playerTime - 2 <= zombieTime) {
                    int dis2 = distance2(player, humans[h]);
                    if (minDis2 == -1 || minDis2 > dis2) {
                        minDis2 = dis2;
                        playerTarget = zombiesNext[zId];
                    }
                }
            }
        }

        return to_string(playerTarget.x) + " " + to_string(playerTarget.y);
    }
};



int main()  {
    while (1) {
        // break;
        State state;
        state.read();
        cout << state.getCommand() << endl;
    }
}





