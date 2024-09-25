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

// for Car simulations doubles are used for precision, to match simulation with the CG's simulation

int gameType = 0;
const int PC = 0;

#define H                       9000.0
#define W                       16000.0
#define PI                      3.14159265358979323846

#define MAX_OUTSIDE_RANGE       100000.0
#define MIN_DIS_TO_CLEAR_CP     600.0

#define MAX_ANGLE_CHANGE        18.0
#define MAX_POWER_CHANGE        200.0
#define MIN_ALLOWED_POWER       30.0
#define MAX_ALLOWED_POWER       200.0


#define GENE_SIZE               600
#define POPULATION_SIZE         50
#define MAX_GENERATIONS         100
#define ELITISM_RATIO           0.2  // 0.1 to 0.2;
#define MUTATION_PROBABILITY    0.01
#define CROSSOVER_PROBABILITY   0.95
#define TOURNAMENT_SIZE         2





std::mt19937 gen(static_cast<unsigned int>(time(NULL)));
int randomInt(int a, int b) { // [a, b]
    std::uniform_int_distribution<> disInt(a, b);
    return disInt(gen);
}
double randomDouble(double a, double b) { // [a, b)
    std::uniform_real_distribution<> disDouble(a, b);
    return disDouble(gen);
}

inline double toDegree(const double radian) { return radian * 180.0 / PI; }
inline double toRadian(const double degree) { return degree * PI / 180.0; }

double getShortestAngle(double currentAngle, double targetAngle) {
    double delta = fmod(targetAngle - currentAngle + 360, 360);
    if (delta > 180) delta -= 360;
    return delta;
}


class Point {
public:
    double x, y;

    Point() : x(0), y(0) {}
    Point(const double& x_, const double& y_) : x(x_), y(y_) {}

    inline Point operator+(const Point& b) const { return Point(x + b.x, y + b.y); }
    inline Point operator-(const Point& b) const { return Point(x - b.x, y - b.y); }
    // inline Point operator*(const int& b) const { return Point(x * b, y * b); }
    // inline Point operator/(const int& b) const { return Point(x / b, y / b); }
    inline Point operator*(const double& b) const { return Point(x * b, y * b); }
    inline Point operator/(const double& b) const { return Point(x / b, y / b); }

    void add(const Point &b) { x += b.x; y += b.y; }
    void subtract(const Point &b) { x -= b.x; y -= b.y; }
    // void multiply(const int &b) { x *= b; y *= b; }
    void multiply(const double &b) { x *= b; y *= b; }
    // void divide(const int &b) { x /= b; y /= b; }
    void divide(const double &b) { x /= b; y /= b; }

    friend double getDistance2(const Point &a, const Point &b) { return (a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y); }
    friend double getDistance(const Point &a, const Point &b) { return sqrtl(double(getDistance2(a, b))); }
    double getDistance(const Point &b) { return sqrtl(double(getDistance2(*this, b))); }

    inline double magnitude() const { return sqrtl(x*x + y*y); }
    inline double sum() const { return x + y; }

    inline double angle() { return toDegree(double(atan(y/x))); }

    // Truncate values (discard decimals)
    void truncate() {
        x = static_cast<int>(x);
        y = static_cast<int>(y);
    }
};


int checkpointsCount;
Point checkpoint[25];


class Car {
public:
    Point pos, prevPos, velo, prevVelo;
    double angle, prevAngle;
    double power;
    int nextCpIdx, reachedCpCount;

    Car(){}

    Car (const Car &car) {
        pos = car.pos; prevPos = car.prevPos; velo = car.velo; prevVelo = car.prevVelo;
        angle = car.angle; prevAngle = car.prevAngle;
        power = car.power;
        nextCpIdx = car.nextCpIdx;
        reachedCpCount = car.reachedCpCount;
    }

    Car(Point &pos_, Point &velo_, double angle_, int &idx) {
        pos = prevPos = pos_; velo = prevVelo = velo_;
        angle = prevAngle = angle_;
        nextCpIdx = idx;
        power = 0;
        reachedCpCount = 0;
    }

    void printState() {
        cerr << "------------------------------------" << endl;
        cerr << "x:     " << round(pos.x) << endl;
        cerr << "y:     " << round(pos.y) << endl;
        cerr << "ux:    " << round(velo.x) << endl;
        cerr << "uy:    " << round(velo.y) << endl;
        cerr << "angle: " << round(angle) << endl;
        cerr << "power: " << round(power) << endl;
        cerr << "cp_id: " << nextCpIdx << endl;
        cerr << "count: " << reachedCpCount << endl;
        cerr << "------------------------------------" << endl;
    }

    bool changeAngleAndPowerBy(const double &angleChange, const double &powerChange) {
        return changeAngleBy(angleChange) && changePowerBy(powerChange);
    }
    bool changeAngleBy(const double &change) {
        prevAngle = angle;
        double ch = change;
        if (fabs(ch) > MAX_ANGLE_CHANGE) {
            ch = (ch > 0) ? MAX_ANGLE_CHANGE : -MAX_ANGLE_CHANGE;
        }
        angle = fmod(angle + ch + 360, 360);  // Ensure angle is between 0-360 degrees
        return true;
    }
    bool changePowerBy(const double &change) {
        power += change;
        if (power > MAX_ALLOWED_POWER) power = MAX_ALLOWED_POWER;
        if (power < MIN_ALLOWED_POWER) power = MIN_ALLOWED_POWER;
        return true;
    }

    bool wentFarOutside() {
        return (
            pos.x < -MAX_OUTSIDE_RANGE || 
            pos.y < -MAX_OUTSIDE_RANGE || 
            pos.x > W + MAX_OUTSIDE_RANGE || 
            pos.y > H + MAX_OUTSIDE_RANGE
        );
    }


    /*
        On each turn the car movement are computed this way:
            1) The car rotates to face the target point, with a maximum of 18 degrees.
            2) The car's facing vector is multiplied by the given thrust value. 
                The result is added to the current speed vector.
            3) The speed vector is added to the position of the car.
            4) The current speed vector is multiplied by 0.85
            5) The speed's values are truncated, angles converted to degrees and rounded 
                and the position's values are truncated.
    */
    void simulate1Step(const double &powerChange) {
        Point target = checkpoint[nextCpIdx];
        double targetAngle = toDegree(atan2(target.y - pos.y, target.x - pos.x));
        double rotation = getShortestAngle(angle, targetAngle);
        changeAngleBy(rotation);

        changePowerBy(powerChange);

        Point facingDirection(cos(toRadian(angle)), sin(toRadian(angle)));
        Point thrustVector = facingDirection * power;

        prevVelo = velo;
        velo.add(thrustVector);

        prevPos = pos;
        pos.add(velo);

        velo.multiply(0.85);

        pos.truncate();
        velo.truncate();
        angle = round(angle);

        // if checkpoint reached -> update Cp related details
        // no two checkpoints overlap otherwise while loop can be used
        if (nextCpIdx < checkpointsCount && getDistance(pos, checkpoint[nextCpIdx]) < MIN_DIS_TO_CLEAR_CP) {
            nextCpIdx++;
            reachedCpCount++;
        }
    }

    double getScore() const {
        double score = 0;
        score += 100000.0 * reachedCpCount;

        if (nextCpIdx < checkpointsCount) {
            double d = getDistance(pos, checkpoint[nextCpIdx]);
            score -= d;
            return score;
        } else {
            // reached all checkpoints -> the further car goes after last checkpoint the more its score will be
            double d = getDistance(pos, checkpoint[checkpointsCount - 1]);
            score += d;
            return score;
        }
    }
};
Car initialCar;



// ======================= Genetic Algorithm =========================================================


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
        angle[i] = randomInt(-MAX_ANGLE_CHANGE, MAX_ANGLE_CHANGE);
        // power[i] = randomInt(-MAX_POWER_CHANGE, MAX_POWER_CHANGE);
        int p[5] = {-200, -100, 0, 100, 200};
        power[i] = p[randomInt(0, 4)];
    }

    void updateFitness() {
        fitness = calculateFitness();
    }

    // calculate fitness by simulation
    float calculateFitness() const {
        Car car(initialCar);
        for (int i = 0; i < GENE_SIZE; i++) {
            car.simulate1Step(power[i]);
            if (car.wentFarOutside()) return 0;
        }
        return car.getScore();
    }

    // only for visualisation
    vector<Point> getPath() const {
        Car car(initialCar);
        vector<Point> path;
        path.emplace_back(car.pos);
        for (int i = 0; i < GENE_SIZE; i++) {
            car.simulate1Step(power[i]);
            path.emplace_back(car.pos);
            if (car.wentFarOutside()) break;
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
    vector<vector<Chromosome>> generations;

    GeneticAlgo () {
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

        // Print checkpoints coordinates on the first line
        for (size_t i = 0; i < checkpointsCount; ++i) {
            outFile << checkpoint[i].x << "," << checkpoint[i].y;
            if (i < checkpointsCount - 1) {
                outFile << " ";  // Separate points by a space
            }
        }
        outFile << "\n";

        // Loop through each generation
        for (const auto& population : generations) {
            // Loop through each chromosome in the generation
            for (const auto& chromosome : population) {
                outFile << "Fitness:" << chromosome.fitness << ";";
                // Loop through each point in the chromosome's path
                for (const auto& point : chromosome.getPath()) {
                    outFile << point.x << "," << point.y << " ";
                }
                outFile << "|";  // Separate chromosomes by a '|'
                break; // save only first chromosome as too many paths to visualise
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
        if (randomDouble(0, 1) < CROSSOVER_PROBABILITY) {
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
        float alpha = randomDouble(0, 1);
        for (int i = 0; i < GENE_SIZE; ++i) {
            offspring1.angle[i] = alpha * p1.angle[i] + (1 - alpha) * p2.angle[i];
            offspring1.power[i] = alpha * p1.power[i] + (1 - alpha) * p2.power[i];

            offspring2.angle[i] = alpha * p2.angle[i] + (1 - alpha) * p1.angle[i];
            offspring2.power[i] = alpha * p2.power[i] + (1 - alpha) * p1.power[i];
        }
    }

    void continousAngleUniformPowerCrossover(const Chromosome &p1, const Chromosome &p2) {
        float alpha = randomDouble(0, 1);
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
        float alpha = randomDouble(0, 1);
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
            if (randomDouble(0, 1) < MUTATION_PROBABILITY) {
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
        //     if (randomDouble(0, 1) < mutationChance) {
        //         chrom.angle[i] = randomInt(-10, 10);
        //         chrom.power[i] = randomInt(-1, 1);
        //     }
        // }
    }

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
    int N, tcIdx;
    vector<Point> cps;
    Point pos, velo;
    double angle;
    int nextCpIdx = 0; // 0 in all testcases
    TestCase(int tNo, int n, vector<Point> cp, Point p, Point v, double a) {
        tcIdx = tNo; N = n; cps = cp; pos = p; velo = v; angle = a;
    }
};
TestCase testCase[19] = {
    TestCase(1,9,{{2757,4659},{3358,2838},{10353,1986},{2757,4659},{3358,2838},{10353,1986},{2757,4659},{3358,2838},{10353,1986}},{10353,1986},{0,0},161),
    TestCase(2,9,{{3431,6328},{4284,2801},{11141,4590},{3431,6328},{4284,2801},{11141,4590},{3431,6328},{4284,2801},{11141,4590}},{11141,4590},{0,0},167),
    TestCase(3,21,{{10892,5399},{4058,1092},{6112,2872},{1961,6027},{7148,4594},{7994,1062},{1711,3942},{10892,5399},{4058,1092},{6112,2872},{1961,6027},{7148,4594},{7994,1062},{1711,3942},{10892,5399},{4058,1092},{6112,2872},{1961,6027},{7148,4594},{7994,1062},{1711,3942}},{1711,3942},{0,0},9),
    TestCase(4,24,{{1043,1446},{10158,1241},{13789,7502},{7456,3627},{6218,1993},{7117,6546},{5163,7350},{12603,1090},{1043,1446},{10158,1241},{13789,7502},{7456,3627},{6218,1993},{7117,6546},{5163,7350},{12603,1090},{1043,1446},{10158,1241},{13789,7502},{7456,3627},{6218,1993},{7117,6546},{5163,7350},{12603,1090}},{12603,1090},{0,0},178),
    TestCase(5,24,{{1271,7171},{14407,3329},{10949,2136},{2443,4165},{5665,6432},{3079,1942},{4019,5141},{9214,6145},{1271,7171},{14407,3329},{10949,2136},{2443,4165},{5665,6432},{3079,1942},{4019,5141},{9214,6145},{1271,7171},{14407,3329},{10949,2136},{2443,4165},{5665,6432},{3079,1942},{4019,5141},{9214,6145}},{9214,6145},{0,0},173),
    TestCase(6,24,{{11727,5704},{11009,3026},{10111,1169},{5835,7503},{1380,2538},{4716,1269},{4025,5146},{8179,7909},{11727,5704},{11009,3026},{10111,1169},{5835,7503},{1380,2538},{4716,1269},{4025,5146},{8179,7909},{11727,5704},{11009,3026},{10111,1169},{5835,7503},{1380,2538},{4716,1269},{4025,5146},{8179,7909}},{8179,7909},{0,0},328),
    TestCase(7,24,{{14908,1849},{2485,3249},{5533,6258},{12561,1063},{1589,6883},{13542,2666},{13967,6917},{6910,1656},{14908,1849},{2485,3249},{5533,6258},{12561,1063},{1589,6883},{13542,2666},{13967,6917},{6910,1656},{14908,1849},{2485,3249},{5533,6258},{12561,1063},{1589,6883},{13542,2666},{13967,6917},{6910,1656}},{6910,1656},{0,0},1),
    TestCase(8,24,{{9882,5377},{3692,3080},{3562,1207},{4231,7534},{14823,6471},{10974,1853},{9374,3740},{4912,4817},{9882,5377},{3692,3080},{3562,1207},{4231,7534},{14823,6471},{10974,1853},{9374,3740},{4912,4817},{9882,5377},{3692,3080},{3562,1207},{4231,7534},{14823,6471},{10974,1853},{9374,3740},{4912,4817}},{4912,4817},{0,0},6),
    TestCase(9,24,{{5874,7746},{7491,4801},{14268,6672},{2796,1751},{1039,2272},{6600,1874},{13467,2208},{13332,4114},{5874,7746},{7491,4801},{14268,6672},{2796,1751},{1039,2272},{6600,1874},{13467,2208},{13332,4114},{5874,7746},{7491,4801},{14268,6672},{2796,1751},{1039,2272},{6600,1874},{13467,2208},{13332,4114}},{13332,4114},{0,0},154),
    TestCase(10,24,{{9623,7597},{12512,6231},{4927,3377},{8358,6630},{4459,7216},{10301,2326},{2145,3943},{5674,4795},{9623,7597},{12512,6231},{4927,3377},{8358,6630},{4459,7216},{10301,2326},{2145,3943},{5674,4795},{9623,7597},{12512,6231},{4927,3377},{8358,6630},{4459,7216},{10301,2326},{2145,3943},{5674,4795}},{5674,4795},{0,0},35),
    TestCase(11,24,{{14203,4266},{3186,5112},{8012,5958},{2554,6642},{5870,4648},{11089,2403},{9144,2389},{12271,7160},{14203,4266},{3186,5112},{8012,5958},{2554,6642},{5870,4648},{11089,2403},{9144,2389},{12271,7160},{14203,4266},{3186,5112},{8012,5958},{2554,6642},{5870,4648},{11089,2403},{9144,2389},{12271,7160}},{12271,7160},{0,0},304),
    TestCase(12,24,{{1779,2501},{5391,2200},{13348,4290},{6144,4176},{11687,5637},{14990,3490},{3569,7566},{14086,1366},{1779,2501},{5391,2200},{13348,4290},{6144,4176},{11687,5637},{14990,3490},{3569,7566},{14086,1366},{1779,2501},{5391,2200},{13348,4290},{6144,4176},{11687,5637},{14990,3490},{3569,7566},{14086,1366}},{14086,1366},{0,0},175),
    TestCase(13,24,{{6419,7692},{2099,4297},{13329,3186},{13870,7169},{13469,1115},{5176,5061},{1260,7235},{9302,5289},{6419,7692},{2099,4297},{13329,3186},{13870,7169},{13469,1115},{5176,5061},{1260,7235},{9302,5289},{6419,7692},{2099,4297},{13329,3186},{13870,7169},{13469,1115},{5176,5061},{1260,7235},{9302,5289}},{9302,5289},{0,0},140),
    TestCase(14,24,{{10177,7892},{5146,7584},{11531,1216},{1596,5797},{8306,3554},{5814,2529},{9471,5505},{6752,5734},{10177,7892},{5146,7584},{11531,1216},{1596,5797},{8306,3554},{5814,2529},{9471,5505},{6752,5734},{10177,7892},{5146,7584},{11531,1216},{1596,5797},{8306,3554},{5814,2529},{9471,5505},{6752,5734}},{6752,5734},{0,0},32),
    TestCase(15,24,{{10312,1696},{2902,6897},{5072,7852},{5918,1004},{3176,2282},{14227,2261},{9986,5567},{9476,3253},{10312,1696},{2902,6897},{5072,7852},{5918,1004},{3176,2282},{14227,2261},{9986,5567},{9476,3253},{10312,1696},{2902,6897},{5072,7852},{5918,1004},{3176,2282},{14227,2261},{9986,5567},{9476,3253}},{9476,3253},{0,0},298),
    TestCase(16,18,{{12000,1000},{12500,2500},{13000,4000},{12500,5500},{12000,7000},{1000,1000},{12000,1000},{12500,2500},{13000,4000},{12500,5500},{12000,7000},{1000,1000},{12000,1000},{12500,2500},{13000,4000},{12500,5500},{12000,7000},{1000,1000}},{1000,1000},{0,0},0),
    TestCase(17,24,{{12500,2500},{12500,5500},{12000,7000},{8000,7000},{7500,5500},{7500,2500},{8000,1000},{12000,1000},{12500,2500},{12500,5500},{12000,7000},{8000,7000},{7500,5500},{7500,2500},{8000,1000},{12000,1000},{12500,2500},{12500,5500},{12000,7000},{8000,7000},{7500,5500},{7500,2500},{8000,1000},{12000,1000}},{12000,1000},{0,0},72),
    TestCase(18,24,{{2500,3905},{4000,5095},{5500,3905},{7000,5095},{8500,3905},{10000,5095},{11500,3905},{1000,4500},{2500,3905},{4000,5095},{5500,3905},{7000,5095},{8500,3905},{10000,5095},{11500,3905},{1000,4500},{2500,3905},{4000,5095},{5500,3905},{7000,5095},{8500,3905},{10000,5095},{11500,3905},{1000,4500}},{1000,4500},{0,0},338),
    TestCase(19,18,{{15000,8000},{1000,8000},{15000,1000},{1000,4500},{15000,4500},{1000,1000},{15000,8000},{1000,8000},{15000,1000},{1000,4500},{15000,4500},{1000,1000},{15000,8000},{1000,8000},{15000,1000},{1000,4500},{15000,4500},{1000,1000}},{1000,1000},{0,0},27)
};
int tcIdx = 0;



void solveOnPc() {
    checkpointsCount = testCase[tcIdx].N;
    for (int i = 0; i < checkpointsCount; i++) {
        checkpoint[i] = testCase[tcIdx].cps[i];
    }
    initialCar = Car(testCase[tcIdx].pos, testCase[tcIdx].velo, testCase[tcIdx].angle, testCase[tcIdx].nextCpIdx);

    GeneticAlgo ga;
    Chromosome ansPath = ga.findSolution();
}

void solveOnSite() {
    cin >> checkpointsCount; cin.ignore();
    for (int i = 0; i < checkpointsCount; i++) {
        cin >> checkpoint[i].x >> checkpoint[i].y; cin.ignore();
    }

    // cerr << checkpointsCount << ",{";
    // for (int i = 0; i < checkpointsCount; i++) {
    //     cerr << "{" << checkpoint[i].x << "," << checkpoint[i].y << "}";
    //     if (i < checkpointsCount - 1) cerr << ",";
    // }

    GeneticAlgo ga;
    Chromosome ansPath;

    for (int turn = 0; ; turn++) {
        int nextCpIdx, angle;
        Point pos, velo;
        cin >> nextCpIdx >> pos.x >> pos.y >> velo.x >> velo.y >> angle;
        if (turn == 0) {
            // cerr << "},{" << pos.x << "," << pos.y << "},{" << velo.x << "," << velo.y << "}," << angle;
            initialCar = Car(pos, velo, angle, nextCpIdx);
            ansPath = ga.findSolution();
        }

        // initialCar.printState();

        // cerr << "Given-------------------------------" << endl;
        // cerr << "x:     " << round(pos.x) << endl;
        // cerr << "y:     " << round(pos.y) << endl;
        // cerr << "ux:    " << round(velo.x) << endl;
        // cerr << "uy:    " << round(velo.y) << endl;
        // cerr << "angle: " << round(angle) << endl;
        // cerr << "------------------------------------" << endl;

        cout << checkpoint[initialCar.nextCpIdx].x << " " << checkpoint[initialCar.nextCpIdx].y << " ";

        double powerChange = ansPath.power[turn];
        initialCar.simulate1Step(powerChange);

        cout << initialCar.power << " " << pos.x << " " << pos.y << endl;
    }
}


int main(){
    srand(time(NULL));

    gameType = PC;
    gameType = 10 - PC;
    if (gameType == PC) {
        solveOnPc();
    } else {
        solveOnSite();
    }
}



