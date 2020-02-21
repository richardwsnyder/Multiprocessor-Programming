#include <algorithm>
#include <atomic>
#include <chrono>
#include <climits>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

using namespace std;

class Lock {
public:
    virtual void lock() = 0;
    virtual void unlock() = 0;
};

class PetersonLock: public Lock {
private:
    vector<thread::id> threadIDs;
    atomic<bool> flag[2];
    atomic<int> victim;
public:
    PetersonLock() {
        victim.store(0, memory_order_relaxed);
        flag[0].store(false, memory_order_release);
        flag[1].store(false, memory_order_release);
    }

    void lock() {
        thread::id me = this_thread::get_id();
        int i;
        vector<thread::id>::iterator it = find(threadIDs.begin(), threadIDs.end(), me);
        // if threadID not in vector, put it in
        if(it == threadIDs.end()) {
            threadIDs.push_back(me);
            it = find(threadIDs.begin(), threadIDs.end(), me);
        }
        i = distance(threadIDs.begin(), it); // this should always be 0 or 1
        int j = 1 - i;
        flag[i].exchange(true, memory_order_release);
        victim.store(i, memory_order_relaxed);
        while(flag[j] && victim == i) {}
    }

    void unlock() {
        thread::id me = this_thread::get_id();
        int i;
        vector<thread::id>::iterator it = find(threadIDs.begin(), threadIDs.end(), me);
        i = distance(threadIDs.begin(), it);
        cout << "thread id " << me << " is now unlocking\n"; 
        flag[i].store(false, memory_order_release);
    }
};

void run(PetersonLock *l) {
    l->lock();
    cout << "hello from inside the run function!\nI'm thread " << this_thread::get_id() << "\n";
    this_thread::sleep_for(chrono::seconds(3));
    l->unlock();
}

int main(void) {
    PetersonLock *l = new PetersonLock();
    cout << "beginning execution...\n";
    thread t1(run, l);
    thread t2(run, l);
    t1.join();
    t2.join();

    return 0;
}
