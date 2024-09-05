#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <math.h>
#include <utility>

using namespace std;


class Point {
public:
    int x, y;

    Point() {}
    Point(const int _x, const int _y) {
        x = _x;
        y = _y;
    }

    bool operator < (Point const &b) const {
        if (x != b.x) return x < b.x;
        return y < b.y;
    }
    bool operator == (Point const &b) const {
        return x == b.x && y == b.y;
    }
    bool operator != (Point const &b) const {
        return x != b.x || y != b.y;
    }

    double dostance_from(const Point &b) {
        return sqrt((x-b.x)*(x-b.x) - (y-b.y)*(y-b.y));
    }
};

double distance(const Point &a, const Point &b) {
    return sqrt((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y));
}


double angle_between_rays(const Point& ray1_start, const Point& ray1_end, const Point& ray2_start, const Point& ray2_end) {
    // Calculate the direction vectors of the rays
    Point ray1_direction(ray1_end.x - ray1_start.x, ray1_end.y - ray1_start.y);
    Point ray2_direction(ray2_end.x - ray2_start.x, ray2_end.y - ray2_start.y);
    
    // Calculate the dot product and cross product of the direction vectors
    double dotProduct = ray1_direction.x * ray2_direction.x + ray1_direction.y * ray2_direction.y;
    double crossProduct = ray1_direction.x * ray2_direction.y - ray1_direction.y * ray2_direction.x;
    
    // Calculate the angle between the rays using the arctangent function
    double angle = atan2(crossProduct, dotProduct);
    
    // Convert the angle from radians to degrees
    angle *= 180.0 / M_PI;
    
    // If the angle is needed in the range [0, 360)
    // if (angle < 0) {
    //     angle += 360.0;
    // }
    
    // return angle;

    // for mad pod coordinate system
    return -angle;
}

// Function to calculate the coordinates of points at a distance D from p1 on the bisector line
pair<Point, Point> find_points_on_bisector(const Point& p1, const Point& p2, const Point& p3, double D) {
    // Calculate unit vectors representing the directions of the lines passing through p1-p2 and p1-p3
    double d12 = distance(p1, p2);
    double d13 = distance(p1, p3);
    double ux = (p2.x - p1.x) / d12;
    double uy = (p2.y - p1.y) / d12;
    double vx = (p3.x - p1.x) / d13;
    double vy = (p3.y - p1.y) / d13;
    
    // Calculate the bisector vector
    double bx = ux + vx;
    double by = uy + vy;
    double len = sqrt(bx * bx + by * by);
    bx /= len;
    by /= len;
    
    // Calculate the coordinates of points on the bisector line at distance D from p1
    Point point1(p1.x + D * bx, p1.y + D * by);
    Point point2(p1.x - D * bx, p1.y - D * by);
    
    return {point1, point2};
}


// Function to calculate the coordinates of point c at distance d from point a and angle 'angle' from line ab
Point calculate_point_at_angle(const Point& a, const Point& b, double d, int angle) {
    double angle_in_degree = M_PI * angle / 180.0;
    double theta = atan2(b.y - a.y, b.x - a.x);
    return Point(a.x + d*cos(theta + angle_in_degree), a.y + d*sin(theta + angle_in_degree));
}

class POD {
public:
    int loop_size;

    vector<Point> path, checkpoints;
    map<Point, bool> cp_map;

    Point pos, dest_pos, cp;
    int thrust;
    int available_boost;
    bool is_boosted;
    double x_speed, y_speed, speed;

    bool looped;
    int cp_index;
    int cp_dist;
    int cp_angle;
    int cp_count;
    int next_cp_angle;
    int cp_speed_angle;
    int face_speed_angle;

    POD() {
        available_boost = 1;
        is_boosted = 0;
        x_speed = y_speed = speed = -1;
        looped = 0;
        cp_count = 0;
        next_cp_angle = -1;
        cp = Point(-1, -1);
    }

    void add_to_path(Point p) {
        path.push_back(p);
    }

    void update_speed() {
        int sz = path.size();
        if (sz > 1) {
            int dist = distance(path[sz-1], path[sz-2]);
            int time = 1;
            speed = dist / time;
            x_speed = (1.0 * (path[sz-1].x - path[sz-2].x) / time);
            y_speed = (1.0 * (path[sz-1].y - path[sz-2].y) / time);
        }
    }

    void set_dest(Point p) {
        dest_pos = p;
    }

    void add_checkpoint(Point _cp) {
        if (cp == _cp) return;

        cp = _cp;
        cp_count++;
        if (!looped) {
            if (cp_map.find(cp) == cp_map.end()) {
                checkpoints.push_back(cp);
                cp_map[cp] = 1;
            } else {
                looped = 1;
                loop_size = checkpoints.size();
                cp_index = 0;
            }
        } else {
            cp_index = (cp_index + 1) % loop_size;
        }
    }

    void use_boost_if_available() {
        if (available_boost > 0 && !is_boosted && cp_dist > 5000 && abs(cp_angle) < 5) {
            is_boosted = 1;
            available_boost--;
        }
    }

    void get_face_speed_angle() {
        Point p = Point(pos.x + x_speed*1000.0, pos.y + y_speed*1000.0);
        cp_speed_angle = angle_between_rays(pos, cp, pos, p);

        // if (cp_speed_angle >= 0) cp_speed_angle = 180 - cp_speed_angle;
        // else cp_speed_angle = -180 - cp_speed_angle;

        cerr << "cp_angle : " << cp_angle << endl;
        cerr << "next_cp_angle : " << next_cp_angle << endl;
        cerr << "cp_speed_angle : " << cp_speed_angle << endl;


        face_speed_angle = cp_speed_angle - cp_angle;
        cerr << "face_speed_angle : " << face_speed_angle << endl;
    }

    void adjust_thrust() {
        get_face_speed_angle();

        // pair<Point, Point> bisector_points = find_points_on_bisector(cp, pos, pos, 400);
        // Point best_bisector_point;
        // if (distance(bisector_points.first, pos) < distance(bisector_points.second, pos)) {
        //     best_bisector_point = bisector_points.first;
        // } else {
        //     best_bisector_point = bisector_points.second;
        // }
        // set_dest(best_bisector_point);

        // double angle = cp_speed_angle > 0 ? 180 - cp_speed_angle : -180 - cp_speed_angle;
        set_dest(calculate_point_at_angle(cp, pos, 400, -cp_speed_angle));

        thrust = 100;

        if (looped) {
            int next_cp_index = (cp_index + 1) % loop_size;

            next_cp_angle = angle_between_rays(
                pos,
                cp,
                cp,
                checkpoints[next_cp_index]
            );

            // pair<Point, Point> bisector_points = find_points_on_bisector(cp, pos, checkpoints[next_cp_index], 400);
            // Point best_bisector_point;
            // if (distance(bisector_points.first, pos) < distance(bisector_points.second, pos)) {
            //     best_bisector_point = bisector_points.first;
            // } else {
            //     best_bisector_point = bisector_points.second;
            // }
            // if (abs(next_cp_angle) > 80) {
            //     set_dest(best_bisector_point);
            // }

            if (abs(cp_angle) < 20 && cp_dist < 1000 && cp_count != 3*loop_size) {
                if (abs(next_cp_angle) < 60) {
                    thrust = 100;
                } else {
                    thrust = max(0.0, 100.0 - (abs(next_cp_angle)));
                }
            }
        }

        if (speed > 500 && cp_dist < 1500) {
            if (abs(cp_angle) > 50) {
                thrust = 0;
            } else if (abs(cp_angle) > 30) {
                thrust = 50;
            }
        }


        if (abs(cp_angle) > 80) {
            thrust = max(0.0, 100.0 - abs(cp_angle));
        }

        if (speed < 300) {
            thrust = 100;
        }
        if (cp_dist < 800) {
            set_dest(cp);
        }
    }

    void run() {
        cout << dest_pos.x << " " << dest_pos.y << " ";
        if (is_boosted) {
            cout << "BOOST";
            is_boosted = 0;
        } else {
            cerr << "Thrust " << thrust << endl;
            cout << thrust;
        }
        // Message
        cout << " s " << speed;
        cout << ", t " << thrust;
        cout << ", sa " << cp_speed_angle;
        cout << ", d " << cp_dist;
        cout << ", cpa " << next_cp_angle;
        cout << endl;
    }
};



int main() {
    POD pod;

    // game loop
    while (1) {
        Point oppo, cp;
        
        cin >> pod.pos.x >> pod.pos.y;
        cin >> cp.x >> cp.y;
        cin >> pod.cp_dist >> pod.cp_angle; cin.ignore();
        cin >> oppo.x >> oppo.y; cin.ignore();


        pod.add_to_path(pod.pos);
        pod.update_speed();

        // for NOW
        pod.set_dest(cp);

        pod.add_checkpoint(cp);

        pod.use_boost_if_available();
        pod.adjust_thrust();
        
        pod.run();
    }
}




















