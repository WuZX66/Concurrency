#include <stdio.h>
#include <iostream>
#include <thread>
#include <queue>
#include <future>
#include <mutex>
#include <stdexcept>
constexpr int WORKERS = 10;
using namespace std;
constexpr int task_nums = 4;

template <typename T>
class SafeQueue
{
private:
    mutex mtx;
    queue<T> task_q;

public:
    SafeQueue()
    {
    }
    void enqueue(T t)
    {
        unique_lock<mutex> lk(mtx);
        task_q.push(t);
    }
    T dequeue()
    {
        if (!this->isEmpty())
        {
            return 0;
        }
        unique_lock<mutex> lk(mtx);
        T t = task_q.front();
        task_q.pop();
        return t;
    }
    int size()
    {
        unique_lock<mutex> lk(mtx);
        int size = task_q.size();
        return size;
    }
    T front()
    {
        T t;
        unique_lock<mutex> lk(mtx);
        t = task_q.front();
        return t;
    }
    T back()
    {
        T t;
        unique_lock<mutex> lk(mtx);
        t = task_q.back();
        return t;
    }
    bool isEmpty()
    {
        unique_lock<mutex> lk(mtx);
        bool empty = task_q.empty();
        return empty;
    }
};
typedef struct task
{
    int (*entry)();
} task;
SafeQueue<int> q;
class ThreadPool
{
private:
    thread workers[WORKERS];
    int cur;
    int avail_workers;
    SafeQueue<task *> task_queue;
    condition_variable full;
    mutex lk;
    bool stop = false;

public:
    ThreadPool()
    {
        this->cur = 0;
        this->avail_workers = WORKERS;
        for (int i = 0; i < WORKERS; ++i)
        {
            workers[i] = thread([this]()
                                {
                                    while (1)
                                    {
                                        task *t = nullptr;
                                        {
                                            unique_lock<mutex> lk(this->lk);
                                            while (this->task_queue.isEmpty() && !this->stop)
                                            {
                                                this->full.wait(lk);
                                            }
                                            if (this->stop && this->task_queue.isEmpty())
                                            {
                                                return;
                                            }
                                            t = this->task_queue.dequeue();
                                        }
                                        t->entry();
                                        delete t;
                                    }
                                });
        }
    }
    future<int> commit(int entry())
    {
        task *t = new task; 
        auto  mytask = make_shared<packaged_task<int()>>(t->entry);
        future<int> res = mytask->get_future();
        t->entry = entry;
        {
            if (this->stop)
            {
                throw runtime_error("enqueue on a stopped pool");
            }
            unique_lock<mutex> lk(this->lk);
            this->task_queue.enqueue(t);
        }
        this->full.notify_one();
        
        return res;
    }
    ~ThreadPool()
    {
        this->stop = true;
        full.notify_all();
        for (int i = 0; i < WORKERS; ++i)
        {
            workers[i].join();
        }
    }
};
int T()
{
    return 1;
}
int main()
{
    thread m[2];

    ThreadPool pool; 
    future<int> res = pool.commit(T);
    res.wait();
    int r = res.get();
    cout << r << endl;
    return 0;
}