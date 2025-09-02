
#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <chrono>
#include <random>

using namespace std;


class TrafficLight {
private:
    string color;
    mutex lightMutex;
public:
    TrafficLight() : color("Red") {}

    void changeLight(string newColor) {
        lock_guard<mutex> lock(lightMutex);
        color = newColor;
    }

    string getLightColor() {
        lock_guard<mutex> lock(lightMutex);
        return color;
    }
};


class Intersection {
private:
    TrafficLight light1, light2;
    queue<int> road1, road2;
    mutex queueMutex;

    int getDynamicTime(int vehicleCount) {
        return 3 + min(vehicleCount, 7); 
    }

public:
    void addVehicle(int road) {
        lock_guard<mutex> lock(queueMutex);
        if (road == 1) road1.push(1);
        else road2.push(1);
    }

    void processTraffic() {
        while (true) {
            int road1Vehicles, road2Vehicles;

            {
                lock_guard<mutex> lock(queueMutex);
                road1Vehicles = road1.size();
                road2Vehicles = road2.size();
            }

            int greenTime1 = getDynamicTime(road1Vehicles);
            int greenTime2 = getDynamicTime(road2Vehicles);

            cout << "Road 1 Green (" << greenTime1 << " sec), Road 2 Red" << endl;
            light1.changeLight("Green");
            light2.changeLight("Red");
            this_thread::sleep_for(chrono::seconds(greenTime1));

            {
                lock_guard<mutex> lock(queueMutex);
                while (!road1.empty()) road1.pop();
            }

            cout << "Road 1 Yellow, Road 2 Red" << endl;
            light1.changeLight("Yellow");
            this_thread::sleep_for(chrono::seconds(2));

            cout << "Road 1 Red, Road 2 Green (" << greenTime2 << " sec)" << endl;
            light1.changeLight("Red");
            light2.changeLight("Green");
            this_thread::sleep_for(chrono::seconds(greenTime2));

            {
                lock_guard<mutex> lock(queueMutex);
                while (!road2.empty()) road2.pop();

                cout << "Road 1 Red, Road 2 Yellow" << endl;
                light2.changeLight("Yellow");
                this_thread::sleep_for(chrono::seconds(2));
            }
        }
    }
};

int main() {
    Intersection intersection;
    thread trafficThread(&Intersection::processTraffic, &intersection);

    
    thread vehicleAdder([&]() {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dist(1, 2);
        while (true) {
            intersection.addVehicle(dist(gen));
            this_thread::sleep_for(chrono::seconds(1));
        }
    });

    trafficThread.join();
    vehicleAdder.join();
    return 0;
}



