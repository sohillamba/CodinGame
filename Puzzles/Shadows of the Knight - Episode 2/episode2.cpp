/*
* inspired by:
* https://github.com/vadim-job-hg/Codingame/blob/master/VERY%20HARD/SHADOW%20OF%20THE%20KNIGHT%20-%20EPISODE%202/java/shadows-of-the-knight-episode-2.java
*/

#include <iostream>
#include <string>

using namespace std;

class Zone {
public:
    int low, high;
    Zone() = default;
    Zone(int low, int high) {
        this->low = low;
        this->high = high;
    }

    void update(Zone z) {
        this->low = z.low;
        this->high = z.high;
    }

    void update(int low, int high) {
        this->low = low;
        this->high = high;
    }
};

Zone warm, cold, current;
int width, height;
int x, y;
int lastX, lastY;
bool foundX=false, firstChance=true, outside=false;
char bombDir = 'U';

void init() {
    current.update(0,width-1);
    cold.update(0,width-1);
    warm.update(0,width-1);
    x = y = lastX = lastY = 0;
}

bool found() {
    int tmpX = x;
    int tmpY = y;
    if (foundX) y = (current.low + current.high)/2;
    else {
        x = (current.low + current.high)/2;
        foundX = true;
        current.update(0, height-1);
        warm.update(current);
        cold.update(current);
    }
    firstChance = true;
    return (x==tmpX && y==tmpY);
}

int get(int value, int limit) {
    int low = current.low;
    int high = current.high;
    int give = low + high - value;
    if (outside) {
        if (value==0) {give = (give-0)/2;}
        else if (value==limit) {give = (limit+give)/2;}
    }
    outside = false;
    if (give==value) give=value+1;
    if (give<0) {give = 0; outside = true;}
    else if (give>limit) {give = limit; outside = true;}
    int middle = (give+value)/2;
    int lower = (give+value-1)/2;
    int higher = (give+value+1)/2;
    if (give>value) {
        warm.update(higher,high);
        cold.update(low,lower);
    }
    else if (give<value) {
        warm.update(low,lower);
        cold.update(higher,high);
    }
    return give;
}

void move() {
    int tmpX = x;
    int tmpY = y;
    switch (bombDir) {
        case 'W':
        current.update(warm);
        break;
        case 'C':
        current.update(cold);
        break;
        case 'S':
        if (!firstChance) {if (!found()) return;}
        break;
    }
    if (current.low >= current.high) {if (!found()) return;}
    firstChance = false;
    if (foundX) y = get(y, height-1);
    else x = get(x, width-1);
    lastX = tmpX;
    lastY = tmpY;
}

int main() {
    cin >> width >> height; cin.ignore();
    init();
    int rounds; cin >> rounds; cin.ignore();
    cin >> x >> y; cin.ignore();
    while (true) {
        string s; cin >> s; cin.ignore(); bombDir = s[0];
        move();
        cout << x << " " << y << endl;
    }
}
