#include <climits>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

using namespace std;

class Counter {
private:
    mutex m;
    long value;
public:
    Counter() {
        value = 0;
    }

    ~Counter() {}

    long getValue() {
        return value;
    }

    long getAndIncrement() {
        m.lock();
        long temp;
        try {
            temp = value;
            value = temp + 1;
        } catch(exception& e) {
            cout << "An exception was thrown with message " << e.what() << "\nExiting and returning INT_MIN\n";
            m.unlock();
            return INT_MIN;
        }
        m.unlock();

        return temp;

    }
};

void incrementCounter(Counter *c, int increment) {
    int i;
    for(i = 0; i < increment; i++)
        c->getAndIncrement();
}

int main(int argc, char **argv) {
    if(argc != 3) {
        cout << "usage:\n./Counter <NUM_THREADS> <INCREMENT>";
        exit(EXIT_FAILURE);
    }
    Counter *c = new Counter();
    int numThreads = stoi(argv[1]), increment = stoi(argv[2]);
    cout << "c initial value: " << c->getValue() << "\n";
    vector<thread> threads;
    int i;
    for(i = 0; i < numThreads; i++)
        threads.push_back(thread(incrementCounter, ref(c), increment));
    for(auto& th : threads)
        th.join();
    cout << "c final value: " << c->getValue() << "\n";

    return 0;
}
