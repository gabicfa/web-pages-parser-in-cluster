#include <iostream>
#include <mutex>
#include <condition_variable>

class Semaphore{
    public:
        Semaphore(int count);
        void acquire();
        void release();
        std::mutex A;
        std::condition_variable cA;

    private:
        int count;  
};

Semaphore::Semaphore(int count):
    count(count){};

void Semaphore::acquire() {
    {
        std::unique_lock<std::mutex> lk(A);
        cA.wait(lk, [&]{return count >0;});
        count = count-1;     
    }
}

void Semaphore::release() {
    {
        std::unique_lock<std::mutex> lk(A);
        count = count + 1;
    }
    cA.notify_one();
}