#include <iostream>
#include <thread>
#include <mutex>
#include <future>
#include <unistd.h>
using namespace std;
int n = 0;
constexpr int P_NUMS = 5;
constexpr int C_NUMS = 5;
mutex mtx;
condition_variable e;
condition_variable full;

int main()
{
    thread p[P_NUMS], c[C_NUMS];
    auto producer = []()
    { while (true)
    {
        unique_lock<mutex> lk(mtx);
        while (n >= 2)
        {
            full.wait(lk);
        }
        cout << "(";
        ++n;
        e.notify_one();
        lk.unlock();
    } };
    auto consumer = []()
    {while (true)
    {
        unique_lock<mutex> lk(mtx);
        while (n <= 0)
        {
            e.wait(lk);
        }
        cout << ")";
        --n;
        full.notify_one();
        lk.unlock();
    } };

    for (int i = 0; i < P_NUMS; ++i)
    {
        p[i] = thread(producer);
    }
    for (int i = 0; i < C_NUMS; ++i)
    {
        c[i] = thread(consumer);
    }
    for (int i = 0; i < P_NUMS; ++i)
    {
        p[i].join();
    }
    for (int i = 0; i < C_NUMS; ++i)
    {
        c[i].join();
    }
    return 0;
}