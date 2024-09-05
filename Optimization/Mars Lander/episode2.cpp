// Genetic algorithm


#pragma GCC optimize("-O3","-ffast-math")
#pragma GCC optimize("inline")
#pragma GCC optimize("omit-frame-pointer")
#pragma GCC optimize("unroll-loops")

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <random>
#include <map>
#include <ctime>
#include <fstream>

using namespace std;

int gameType = 0;
const int PC = 0;

const int H = 3000;
const int W = 7000;
const float G = -3.711;
const float PI = 3.14159265358979323846;

std::mt19937 gen(static_cast<unsigned int>(time(NULL)));

int randomInt(int a, int b) { // [a, b]
    std::uniform_int_distribution<> disInt(a, b);
    return disInt(gen);

    // return a + rand() % (b - a + 1);
}
float randomFloat(float a, float b) { // [a, b)
    std::uniform_real_distribution<> disFloat(a, b);
    return disFloat(gen);
}

// ================== 2 D ===============================
struct Point {
    float x, y;
    Point() {}
    Point(float a, float b) {
        x = a; y = b;
    }
};

struct Segment {
    Point a, b;
    Segment(){}
    Segment(Point x, Point y) {
        a = x; b = y;
    }
};

int landN;
Point land[31];
Segment landingZone;
int landingZoneIdx; // landingZine = landingZoneIdx-1 to landingZoneIdx
float distanceOfSegment[31];



// Function to calculate orientation of the triplet (p, q, r)
int orientation(const Point &p, const Point &q, const Point &r) {
    float val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
    if (val == 0) return 0; // Collinear
    return (val > 0) ? 1 : 2; // Clockwise or Counterclockwise
}

// Function to check if point q lies on line segment pr
bool onSegment(const Point &p, const Point &q, const Point &r) {
    if (q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x) &&
        q.y <= std::max(p.y, r.y) && q.y >= std::min(p.y, r.y))
        return true;
    return false;
}

// Function to check if line segments AB and CD intersect
bool doIntersect(const Point &A, const Point &B, const Point &C, const Point &D) {
    int o1 = orientation(A, B, C);
    int o2 = orientation(A, B, D);
    int o3 = orientation(C, D, A);
    int o4 = orientation(C, D, B);

    if (o1 != o2 && o3 != o4)
        return true;

    if (o1 == 0 && onSegment(A, C, B)) return true;
    if (o2 == 0 && onSegment(A, D, B)) return true;
    if (o3 == 0 && onSegment(C, A, D)) return true;
    if (o4 == 0 && onSegment(C, B, D)) return true;

    return false;
}

// Function to check if line segment AB intersects the boundary formed by points
// and if true the return the index of the 2nd point of the segment with it intersects
int getIntersectingBoundaryIndex(const Point &A, const Point &B) {
    for (int i = 1; i < landN; ++i) {
        if (doIntersect(A, B, land[i-1], land[i])) {
            return i;
        }
    }
    return -1;
}

// ========

// Function to compute the dot product of two vectors
float dotProduct(const Point& v1, const Point& v2) {
    return v1.x * v2.x + v1.y * v2.y;
}

// Function to subtract two points to get a vector
Point vectorSubtract(const Point& p1, const Point& p2) {
    return {p1.x - p2.x, p1.y - p2.y};
}

// Function to compute the squared magnitude of a vector
float magnitudeSquared(const Point& v) {
    return v.x * v.x + v.y * v.y;
}

// Function to compute the Euclidean distance between two points
float distance(const Point& p1, const Point& p2) {
    // return std::sqrt(magnitudeSquared(vectorSubtract(p1, p2)));
    return std::sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}

// Function to find the shortest distance from point P to line segment AB
float shortestDistanceToSegment(const Point &A, const Point &B, const Point &P) {
    Point AP = vectorSubtract(P, A);
    Point AB = vectorSubtract(B, A);

    float AB_magSq = magnitudeSquared(AB);
    float t = dotProduct(AP, AB) / AB_magSq;

    if (t < 0.0) {
        // Closest to A
        return distance(P, A);
    } else if (t > 1.0) {
        // Closest to B
        return distance(P, B);
    } else {
        // Closest to a point on the segment
        Point closestPoint = {A.x + t * AB.x, A.y + t * AB.y};
        return distance(P, closestPoint);
    }
}

// ======================== 2 D ends ===================

const float MAX_X_SPEED = 18;
const float MAX_Y_SPEED = 38;


Point getAccln(const int &angle, const int &power) {
    return Point(1.0 * power * sin(-1.0 * angle * PI / 180.0), 1.0 * power * cos(1.0 * angle * PI / 180.0) + G);
}

class Shuttle {
public:
    Point pos, prevPos, velo, prevVelo;
    int angle, prevAngle, power, fuel, initFuel;

    Shuttle(){}

    Shuttle (const Shuttle &sh) {
        pos = sh.pos; prevPos = sh.prevPos; velo = prevVelo = sh.velo;
        angle = prevAngle = sh.angle; power = sh.power; fuel = sh.fuel; initFuel = sh.initFuel;
    }

    Shuttle(Point &pos_, Point &velo_, int &angle_, int &power_, int &fuel_) {
        pos = prevPos = pos_; velo = prevVelo = velo_;
        angle = prevAngle = angle_; power = power_; fuel = initFuel = fuel_;
    }

    void printState() {
        cerr << "------------------------------------" << endl;
        cerr << "x:     " << round(pos.x) << endl;
        cerr << "y:     " << round(pos.y) << endl;
        cerr << "ux:    " << round(velo.x) << endl;
        cerr << "uy:    " << round(velo.y) << endl;
        cerr << "angle: " << angle << endl;
        cerr << "power: " << power << endl;
        cerr << "fuel:  " << fuel << endl;
        cerr << "------------------------------------" << endl;
    }

    bool changeAngleAndPowerBy(const int &angleChange, const int &powerChange) {
        return changeAngleBy(angleChange) && changePowerBy(powerChange);
    }
    bool changeAngleBy(const int &change) {
        // if (abs(angle + change) > 90) return false;
        prevAngle = angle;
        angle += change;
        if (angle > 90) angle = 90;
        if (angle < -90) angle = -90;
        return true;
    }
    bool changePowerBy(const int &change) {
        // if (power + change > 4 || power + change < 0) return false;
        power += change;
        if (power > 4) power = 4;
        if (power < 0) power = 0;
        return true;
    }

    void simulate1Step() {
        prevPos = pos;
        prevVelo = velo;

        if (fuel <= 0) {
            power = 0;
        }
        Point accl = getAccln(angle, power);

        pos.x += 1.0 * velo.x + 0.5 * accl.x;
        pos.y += 1.0 * velo.y + 0.5 * accl.y;

        velo.x = 1.0 * velo.x + accl.x;
        velo.y = 1.0 * velo.y + accl.y;

        fuel -= power;
    }

    bool isOutside() const {
        return pos.x < 0 || pos.x >= W || pos.y < 0 || pos.y >= H;
    }

    int getCollisionId() const {
        return getIntersectingBoundaryIndex(prevPos, pos);
    }


    float getDistanceToLandingArea(const Point &p) const {
        return shortestDistanceToSegment(landingZone.a, landingZone.b, p);
    }

    float getDistanceToLandingAreaUsingBoundryPath(const Point &p, const int &collisionId) const {
        if (distanceOfSegment[collisionId] < distanceOfSegment[collisionId-1]) {
            return distanceOfSegment[collisionId] + distance(p, land[collisionId]);
        } else {
            return distanceOfSegment[collisionId-1] + distance(p, land[collisionId-1]);
        }
    }

    float getScore(const bool reached, const int &collisionId = -1) const {
        float currentSpeed = sqrt(velo.x * velo.x + velo.y * velo.y);
        float score = 0;

        float xSpeed = max(abs(velo.x), abs(prevVelo.x));
        float ySpeed = max(abs(velo.y), abs(prevVelo.y));
        int curAngle = max(abs(angle), abs(prevAngle));

        // 0-100: crashed somewhere, calculate score by distance to landing area
        if (!reached) {
            float dist = getDistanceToLandingArea(prevPos);

            if (collisionId != -1) {
                dist = getDistanceToLandingAreaUsingBoundryPath(prevPos, collisionId);
            }

            // Calculate score from dist
            score = 100.0 - (100.0 * dist / 7616.0);

            // High speeds are bad, they decrease maneuvrability
            float speedPen = 0.1 * max(currentSpeed - 100, 0.0f);
            score -= speedPen;
            return score;
        }


        // 100-200: crashed into landing area, calculate score by speed above safety or angle above safety
        // else if (xSpeed > MAX_X_SPEED || ySpeed > MAX_Y_SPEED || curAngle > 0) {
        //     float xPen = 0, yPen = 0, anglePen = 0;
        //     if (xSpeed > MAX_X_SPEED) {
        //         xPen = (xSpeed - MAX_X_SPEED) / MAX_X_SPEED * 25.0;
        //     }
        //     if (ySpeed > MAX_Y_SPEED) {
        //         yPen = (ySpeed - MAX_Y_SPEED) / MAX_Y_SPEED * 25.0;
        //     }
        //     if (curAngle) {
        //         anglePen = 1.0 * curAngle / 90.0 * 50.0;
        //     }
        //     return score = 200.0 - max(xPen + yPen + anglePen, 0.0f);
        // }
        // else if (xSpeed > MAX_X_SPEED || ySpeed > MAX_Y_SPEED) {
        //     float xPen = 0, yPen = 0, anglePen = 0;
        //     if (xSpeed > MAX_X_SPEED) {
        //         xPen = (xSpeed - MAX_X_SPEED) / 2.0;
        //     }
        //     if (ySpeed > MAX_Y_SPEED) {
        //         yPen = (ySpeed - MAX_Y_SPEED) / 2.0;
        //     }
        //     return score = 200.0 - xPen - yPen - anglePen;
        // }

        // 100-300: crashed into landing area: give score based on speeds and angle
        else if (xSpeed > MAX_X_SPEED || ySpeed > MAX_Y_SPEED || curAngle > 0) {
            // 100 for x and y speed
            float xPen = 0, yPen = 0;
            if (xSpeed > MAX_X_SPEED) {
                xPen = (xSpeed - MAX_X_SPEED) / 2.0;
            }
            if (ySpeed > MAX_Y_SPEED) {
                yPen = (ySpeed - MAX_Y_SPEED) / 2.0;
            }

            // if speeds are safe -> 100 for angle
            if (xSpeed <= MAX_X_SPEED && ySpeed <= MAX_Y_SPEED) {
                float anglePen = curAngle;
                return 300.0 - min(anglePen, 100.0f);
            } else {
                return 200.0 - min(xPen + yPen, 100.0f);
            }
        }

        // 300-400: landed safely, calculate score by fuel remaining
        else {
            return 300.0 + (100.0 * fuel / initFuel);
        }
    }
};
Shuttle startingShuttle;



// ======================= Genetic Algorithm =========================================================

const int GENE_SIZE = 100;
const int POPULATION_SIZE = 100;
const int MAX_GENERATIONS = 420;
const float ELITISM_RATIO = 0.2; // 0.1 to 0.2;
const float MUTATION_PROBABILITY = 0.01;
const float CROSSOVER_PROBABILITY = 0.95;
const int TOURNAMENT_SIZE = 2;



class Chromosome {
public:
    int *angle;
    int *power;
    float fitness = 0;

    Chromosome() {
        angle = new int[GENE_SIZE];
        power = new int[GENE_SIZE];
        for (int i = 0; i < GENE_SIZE; i++) {
            ramdomizeIthGene(i);
        }
    }

    // Copy constructor for deep copy
    Chromosome(const Chromosome& other) {
        angle = new int[GENE_SIZE];
        power = new int[GENE_SIZE];
        std::copy(other.angle, other.angle + GENE_SIZE, angle);
        std::copy(other.power, other.power + GENE_SIZE, power);
        fitness = other.fitness;
    }

    // Assignment operator for deep copy
    Chromosome& operator=(const Chromosome& other) {
        if (this != &other) {
            delete[] angle;
            delete[] power;

            angle = new int[GENE_SIZE];
            power = new int[GENE_SIZE];
            std::copy(other.angle, other.angle + GENE_SIZE, angle);
            std::copy(other.power, other.power + GENE_SIZE, power);
            fitness = other.fitness;
        }
        return *this;
    }

    ~Chromosome() {
        delete[] angle;
        delete[] power;
    }

    void ramdomizeIthGene(const int &i) {
        angle[i] = randomInt(-15, 15);
        power[i] = randomInt(-1, 1);
    }

    void updateFitness() {
        fitness = getFitness();
    }

    float getFitness() const {
        Shuttle sh = Shuttle(startingShuttle);
        for (int i = 0; i < GENE_SIZE; i++) {
            if (!sh.changeAngleAndPowerBy(angle[i], power[i])) {
                return sh.getScore(false); // invalid angle or power
            }
            
            sh.simulate1Step();
            if (sh.isOutside()) {
                return sh.getScore(false); // shuttle went outside
            }
            
            int collisionId = sh.getCollisionId();
            if (collisionId == -1) {
                continue;
            }
            if (collisionId != landingZoneIdx) {
                return sh.getScore(false, collisionId); // crashed
            }

            // reached landing zone -> check other requirments
            return sh.getScore(true); // reached = true
        }
        return sh.getScore(false); // reached = false
    }

    // only for visualisation
    vector<Point> getPath() const {
        vector<Point> path;
        Shuttle sh = Shuttle(startingShuttle);
        path.emplace_back(sh.pos);
        for (int i = 0; i < GENE_SIZE; i++) {
            if (!sh.changeAngleAndPowerBy(angle[i], power[i])) {
                return path; // invalid angle or power
            }
            
            sh.simulate1Step();
            path.emplace_back(sh.pos);
            if (sh.isOutside()) {
                return path; // shuttle went outside
            }
            
            int collisionId = sh.getCollisionId();
            if (collisionId == -1) {
                continue; // no collision with anything
            }
            if (collisionId != landingZoneIdx) {
                return path; // crashed
            }

            // reached landing zone
            return path;
        }
        return path;
    }
};

class GeneticAlgo {
public:
    Chromosome *pop, *newPop;
    Chromosome offspring1, offspring2;
    // float *cumulativeSum;
    int newPopSize;

    // only for visualisation
    vector<Point> ground;
    vector<vector<Chromosome>> generations;

    GeneticAlgo (const vector<Point> &_ground = {}) {
        pop = new Chromosome[POPULATION_SIZE];
        newPop = new Chromosome[POPULATION_SIZE];
        // cumulativeSum = new float[POPULATION_SIZE];
        for (int i = 0; i < POPULATION_SIZE; i++) {
            pop[i] = Chromosome();
            pop[i].updateFitness();
        }
        sort(pop, pop + POPULATION_SIZE, [&](const Chromosome &ch1, const Chromosome &ch2) {
            return ch1.fitness > ch2.fitness;
        });
        if (gameType == PC) {
            generations.clear();
            ground = _ground;
        }
    }

    ~GeneticAlgo() {
        delete[] pop;
        delete[] newPop;
        // delete[] cumulativeSum;
    }

    // only for visualisation
    void savePaths() {
        ofstream outFile("paths.txt");

        // Print ground coordinates on the first line
        for (size_t i = 0; i < ground.size(); ++i) {
            outFile << ground[i].x << "," << ground[i].y;
            if (i < ground.size() - 1) {
                outFile << " ";  // Separate points by a space
            }
        }
        outFile << "\n";  // End the ground coordinates line

        // Loop through each generation
        for (const auto& population : generations) {
            // Loop through each chromosome in the generation
            for (const auto& chromosome : population) {
                outFile << "Fitness:" << chromosome.getFitness() << ";";
                // Loop through each point in the chromosome's path
                for (const auto& point : chromosome.getPath()) {
                    outFile << point.x << "," << point.y << " ";
                }
                outFile << "|";  // Separate chromosomes by a '|'
            }
            outFile << "\n";  // Separate generations by a newline
        }

        outFile.close();
    }

    Chromosome findSolution() {
        Chromosome ans = pop[0];
        for (int depth = 0; depth < MAX_GENERATIONS; depth++) {
            generation();
            if (pop[0].fitness > ans.fitness) {
                ans = pop[0];
            }
            if (gameType == PC) generations.emplace_back(vector<Chromosome>(pop, pop + POPULATION_SIZE)); // only for visualisation
        }
        if (gameType == PC) savePaths();
        return ans;
    }

    void generation () {
        newPopSize = 0;

        select();
        
        // fill cumulativeSum
        // cumulativeSum[0] = pop[0].fitness;
        // for (int i = 1; i < POPULATION_SIZE; i++) {
        //     cumulativeSum[i] = cumulativeSum[i-1] + pop[i].fitness;
        // }

        while (newPopSize < POPULATION_SIZE) {
            int p1 = tournamentSelection();
            int p2 = tournamentSelection();
            while (p2 == p1) {
                p2 = tournamentSelection();
            }
            crossover(pop[p1], pop[p2]);

            mutate(offspring1, pop[p1], pop[p2]);
            offspring1.updateFitness();
            newPop[newPopSize++] = offspring1;
            
            if (newPopSize < POPULATION_SIZE) {
                mutate(offspring2, pop[p1], pop[p2]);
                offspring2.updateFitness();
                newPop[newPopSize++] = offspring2;
            }
        }

        swapPopulations();
        std::sort(pop, pop + POPULATION_SIZE, [&](const Chromosome &ch1, const Chromosome &ch2) {
            return ch1.fitness > ch2.fitness;
        });
    }

    void select() {
        for (int i = 0; i < POPULATION_SIZE * ELITISM_RATIO; i++) {
            newPop[newPopSize++] = pop[i];
        }
    }

    void crossover (const Chromosome &p1, const Chromosome &p2) {
        if (randomFloat(0, 1) < CROSSOVER_PROBABILITY) {
            // continousCrossover(p1, p2);
            // continousAngleUniformPowerCrossover(p1, p2);
            uniforCrossover(p1, p2);
            // singlePointCrossover(p1, p2);
            // twoPointCrossOver(p1, p2);
            // blendedAngleUniformPowerCrossover(p1, p2);
        } else {
            offspring1 = p1;
            offspring2 = p2;
        }
    }

    void continousCrossover(const Chromosome &p1, const Chromosome &p2) {
        float alpha = randomFloat(0, 1);
        for (int i = 0; i < GENE_SIZE; ++i) {
            offspring1.angle[i] = alpha * p1.angle[i] + (1 - alpha) * p2.angle[i];
            offspring1.power[i] = alpha * p1.power[i] + (1 - alpha) * p2.power[i];

            offspring2.angle[i] = alpha * p2.angle[i] + (1 - alpha) * p1.angle[i];
            offspring2.power[i] = alpha * p2.power[i] + (1 - alpha) * p1.power[i];
        }
    }

    void continousAngleUniformPowerCrossover(const Chromosome &p1, const Chromosome &p2) {
        float alpha = randomFloat(0, 1);
        for (int i = 0; i < GENE_SIZE; ++i) {
            offspring1.angle[i] = alpha * p1.angle[i] + (1 - alpha) * p2.angle[i];
            offspring2.angle[i] = alpha * p2.angle[i] + (1 - alpha) * p1.angle[i];
            if (randomInt(0, 1) == 0) {
                offspring1.power[i] = p1.power[i];
                offspring2.power[i] = p2.power[i];
            } else {
                offspring1.power[i] = p2.power[i];
                offspring2.power[i] = p1.power[i];
            }
        }
    }

    void uniforCrossover(const Chromosome &p1, const Chromosome &p2) {
        for (int i = 0; i < GENE_SIZE; i++) {
            if (randomInt(0, 1) == 0) {
                offspring1.angle[i] = p1.angle[i];
                offspring1.power[i] = p1.power[i];

                offspring2.angle[i] = p2.angle[i];
                offspring2.power[i] = p2.power[i];
            } else {
                offspring1.angle[i] = p2.angle[i];
                offspring1.power[i] = p2.power[i];

                offspring2.angle[i] = p1.angle[i];
                offspring2.power[i] = p1.power[i];
            }
        }
    }

    void singlePointCrossover(const Chromosome &p1, const Chromosome &p2) {
        int crossoverPoint = randomInt(0, GENE_SIZE - 1);
        for (int i = 0; i < GENE_SIZE; i++) {
            if (i <= crossoverPoint) {
                offspring1.angle[i] = p1.angle[i];
                offspring1.power[i] = p1.power[i];

                offspring2.angle[i] = p2.angle[i];
                offspring2.power[i] = p2.power[i];
            } else {
                offspring1.angle[i] = p2.angle[i];
                offspring1.power[i] = p2.power[i];

                offspring2.angle[i] = p1.angle[i];
                offspring2.power[i] = p1.power[i];
            }
        }
    }

    void twoPointCrossOver(const Chromosome &p1, const Chromosome &p2) {
        int crossoverPoint1 = randomInt(0, GENE_SIZE - 2);
        int crossoverPoint2 = randomInt(crossoverPoint1 + 1, GENE_SIZE - 1);
        for (int i = 0; i < GENE_SIZE; i++) {
            if (i <= crossoverPoint1 || i > crossoverPoint2) {
                offspring1.angle[i] = p1.angle[i];
                offspring1.power[i] = p1.power[i];

                offspring2.angle[i] = p2.angle[i];
                offspring2.power[i] = p2.power[i];
            } else {
                offspring1.angle[i] = p2.angle[i];
                offspring1.power[i] = p2.power[i];

                offspring2.angle[i] = p1.angle[i];
                offspring2.power[i] = p1.power[i];
            }
        }
    }

    void blendedAngleUniformPowerCrossover(const Chromosome &p1, const Chromosome &p2) {
        float alpha = randomFloat(0, 1);
        for (int i = 0; i < GENE_SIZE; i++) {
            float lower = min(p1.angle[i], p2.angle[i]);
            float upper = max(p1.angle[i], p2.angle[i]);

            offspring1.angle[i] = lower + static_cast<int>((upper - lower + 1) * ((1 + alpha) * rand() / RAND_MAX) - alpha);
            offspring2.angle[i] = lower + static_cast<int>((upper - lower + 1) * ((1 + alpha) * rand() / RAND_MAX) - alpha);

            if (randomInt(0, 1) == 0) {
                offspring1.power[i] = p1.power[i];
                offspring2.power[i] = p2.power[i];
            } else {
                offspring1.power[i] = p2.power[i];
                offspring2.power[i] = p1.power[i];
            }
        }
    }

    void mutate (Chromosome &chrom, const Chromosome &p1, const Chromosome &p2) {
        for (int i = 0; i < GENE_SIZE; i++) {
            if (randomFloat(0, 1) < MUTATION_PROBABILITY) {
                chrom.ramdomizeIthGene(i);
            }
        }
        // for (int i = 0; i < GENE_SIZE; i++) {
        //     // Better parent score -> Lower mutation
        //     // Later gene -> Higher mutation
        //     float scoreMultiplier = 5;
        //     if (100 < max(p1.fitness, p2.fitness)) {
        //         scoreMultiplier = 3;
        //     }
        //     else if (300 < max(p1.fitness, p2.fitness)) {
        //         scoreMultiplier = 1;
        //     }
        //     float progress = (float) i / GENE_SIZE;
        //     float progressChance = 0.4f + 1.0f * progress + 10.0f * progress * progress;
        //     float mutationChance = 0.05f * scoreMultiplier * progressChance;
        //     if (randomFloat(0, 1) < mutationChance) {
        //         chrom.angle[i] = randomInt(-10, 10);
        //         chrom.power[i] = randomInt(-1, 1);
        //     }
        // }
    }

    // int rouletteWheelSelection() {
    //     // return randomInt(0, POPULATION_SIZE - 1);
    //     float randomValue = randomFloat(0.0, cumulativeSum[POPULATION_SIZE - 1]);
    //     auto it = std::lower_bound(cumulativeSum, cumulativeSum + POPULATION_SIZE, randomValue);
    //     return it - cumulativeSum;
    // }

    int tournamentSelection() const {
        int best = -1;
        for (int i = 0; i < TOURNAMENT_SIZE; ++i) {
            int randIndex = randomInt(0, POPULATION_SIZE - 1);
            if (best == -1 || pop[randIndex].fitness > pop[best].fitness) {
                best = randIndex;
            }
        }
        return best;
    }

    void swapPopulations() {
        std::swap(pop, newPop);
    }
};


// ===================================================================================================


struct TestCase {
    int N;
    vector<Point> L;
    double X, Y, UX, UY;
    int fuel, angle, power;
    TestCase(int n, vector<Point> l, double x, double y, double ux, double uy, int f, int a, int p) {
        N = n;
        L = l;
        X = x; Y = y; UX = ux; UY = uy;
        fuel = f; angle = a; power = p;
    }
};
TestCase testCase[5] = {
    TestCase(7, {{0,100}, {1000,500}, {1500,1500}, {3000,1000}, {4000,150}, {5500,150}, {6999,800}}, 2500, 2700, 0, 0, 550, 0, 0),
    TestCase(10, {{0,100}, {1000,500}, {1500,100}, {3000,100}, {3500,500}, {3700,200}, {5000,1500}, {5800,300}, {6000,1000}, {6999,2000}}, 6500, 2800, -100, 0, 600, 90, 0),
    TestCase(7, {{0,100}, {1000,500}, {1500,1500}, {3000,1000}, {4000,150}, {5500,150}, {6999,800}}, 6500, 2800, -90, 0, 750, 90, 0),
    TestCase(20, {{0,1000}, {300,1500}, {350,1400}, {500,2000}, {800,1800}, {1000,2500}, {1200,2100}, {1500,2400}, {2000,1000}, {2200,500}, {2500,100}, {2900,800}, {3000,500}, {3200,1000}, {3500,2000}, {3800,800}, {4000,200}, {5000,200}, {5500,1500}, {6999,2800}}, 500, 2700, 100, 0, 800, -90, 0),
    TestCase(20, {{0,1000}, {300,1500}, {350,1400}, {500,2100}, {1500,2100}, {2000,200}, {2500,500}, {2900,300}, {3000,200}, {3200,1000}, {3500,500}, {3800,800}, {4000,200}, {4200,800}, {4800,600}, {5000,1200}, {5500,900}, {6000,500}, {6500,300}, {6999,500}}, 6500, 2700, -50, 0, 1000, 90, 0)
};
int currentTestCaseIndex = 4;



void solveOnPc() {
    // cin >> landN; cin.ignore();
    // for (int i = 0; i < landN; i++) {
    //     cin >> land[i].x >> land[i].y; cin.ignore();
    //     if (i > 0 && land[i].y == land[i-1].y && abs(land[i].x - land[i-1].x + 1) >= 1000) {
    //         landingZone = Segment(land[i-1], land[i]);
    //         landingZoneIdx = i;
    //     }
    // }

    // cout << landN << ", {";
    // for (int i = 0; i < landN; i++) {
    //     cout << "{" << land[i].x << "," << land[i].y << "}";
    //     if (i < landN - 1) {
    //         cout << ", ";
    //     }
    // }
    // cout << "}, ";

    landN = testCase[currentTestCaseIndex].N;
    for (int i = 0; i < landN; i++) {
        land[i] = testCase[currentTestCaseIndex].L[i];
        if (i > 0 && land[i].y == land[i-1].y && abs(land[i].x - land[i-1].x + 1) >= 1000) {
            landingZone = Segment(land[i-1], land[i]);
            landingZoneIdx = i;
        }
    }

    distanceOfSegment[landingZoneIdx] = distanceOfSegment[landingZoneIdx-1] = 0;
    for (int i = landingZoneIdx - 2; i >= 0; i--) {
        distanceOfSegment[i] = distanceOfSegment[i+1] + distance(land[i], land[i+1]);
    }
    for (int i = landingZoneIdx + 1; i < landN; i++) {
        distanceOfSegment[i] = distanceOfSegment[i-1] + distance(land[i], land[i-1]);
    }

    GeneticAlgo GA(testCase[currentTestCaseIndex].L);
    Chromosome ans;

    int turn = 0;
    while (1) {
        float x, y, ux, uy;
        int fuel, angle, power;
        x = testCase[currentTestCaseIndex].X;
        y = testCase[currentTestCaseIndex].Y;
        ux = testCase[currentTestCaseIndex].UX;
        uy = testCase[currentTestCaseIndex].UY;
        fuel = testCase[currentTestCaseIndex].fuel;
        angle = testCase[currentTestCaseIndex].angle;
        power = testCase[currentTestCaseIndex].power;
        // cin >> x >> y >> ux >> uy >> fuel >> angle >> power; cin.ignore();
        // cout << x << ", " << y << ", " << ux << ", " << uy << ", " << fuel << ", " << angle << ", " << power;

        if (turn == 0) {
            Point pos(x, y), velo(ux, uy);
            startingShuttle = Shuttle(pos, velo, angle, power, fuel);

            ans = GA.findSolution();
            for (int i = 0; i < GENE_SIZE; i++) {
                startingShuttle.changeAngleAndPowerBy(ans.angle[i], ans.power[i]);
                startingShuttle.simulate1Step();
                // startingShuttle.printState();
            }
        }

        // cerr << "x:     " << round(startingShuttle.pos.x) << endl;
        // cerr << "y:     " << round(startingShuttle.pos.y) << endl;
        // cerr << "ux:    " << round(startingShuttle.velo.x) << endl;
        // cerr << "uy:    " << round(startingShuttle.velo.y) << endl;
        // cerr << "angle: " << startingShuttle.angle << endl;
        // cerr << "power: " << startingShuttle.power << endl;
        // cerr << "fuel:  " << startingShuttle.fuel << endl;

        // cerr << "prev angle " << angle << endl;
        // cerr << "prev power " << power << endl;

        turn++;
        break;
    }
}

void solveOnSite() {
    cin >> landN; cin.ignore();
    for (int i = 0; i < landN; i++) {
        cin >> land[i].x >> land[i].y; cin.ignore();
        if (i > 0 && land[i].y == land[i-1].y && abs(land[i].x - land[i-1].x + 1) >= 1000) {
            landingZone = Segment(land[i-1], land[i]);
            landingZoneIdx = i;
        }
    }

    distanceOfSegment[landingZoneIdx] = distanceOfSegment[landingZoneIdx-1] = 0;
    for (int i = landingZoneIdx - 2; i >= 0; i--) {
        distanceOfSegment[i] = distanceOfSegment[i+1] + distance(land[i], land[i+1]);
    }
    for (int i = landingZoneIdx + 1; i < landN; i++) {
        distanceOfSegment[i] = distanceOfSegment[i-1] + distance(land[i], land[i-1]);
    }

    GeneticAlgo GA;
    Chromosome ans;

    int turn = 0;
    while (1) {
        float x, y, ux, uy;
        int fuel, angle, power;
        cin >> x >> y >> ux >> uy >> fuel >> angle >> power; cin.ignore();

        if (turn == 0) {
            // if (1) {
            //     cerr << landN << endl;
            //     for (int i = 0; i < landN; i++) {
            //         cerr << land[i].x << " " << land[i].y << endl;
            //     }
            //     cerr << x << " " << y << " " << ux << " " << uy << " " << fuel << " " << angle << " " << power << endl;
            // }
            // cerr << "Input printing done!" << endl;

            Point pos(x, y), velo(ux, uy);
            startingShuttle = Shuttle(pos, velo, angle, power, fuel);

            ans = GA.findSolution();
        }

        bool isValid = startingShuttle.changeAngleAndPowerBy(ans.angle[turn], ans.power[turn]);
        if (!isValid) {
            cerr << "Invalid change occured!" << endl;
        }

        float dis = startingShuttle.getDistanceToLandingArea(startingShuttle.pos);
        cerr << dis << endl;
        if (dis < 100) {
            cout << 0 << " " << 4 << endl;
        } else {
            startingShuttle.simulate1Step();
            cout << startingShuttle.angle << " " << startingShuttle.power << endl;
        }

        // cerr << "x:     " << round(startingShuttle.pos.x) << endl;
        // cerr << "y:     " << round(startingShuttle.pos.y) << endl;
        // cerr << "ux:    " << round(startingShuttle.velo.x) << endl;
        // cerr << "uy:    " << round(startingShuttle.velo.y) << endl;
        // cerr << "angle: " << startingShuttle.angle << endl;
        // cerr << "power: " << startingShuttle.power << endl;
        // cerr << "fuel:  " << startingShuttle.fuel << endl;

        // cerr << "prev angle " << angle << endl;
        // cerr << "prev power " << power << endl;

        turn++;
    }
}


int main(){
    srand(time(NULL));

    // gameType = PC;
    gameType = 10 - PC;
    if (gameType == PC) {
        solveOnPc();

    } else {
        solveOnSite();
    }
}



