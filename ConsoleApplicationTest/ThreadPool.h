#ifndef TP_THREADPOOL_H
#define TP_THREADPOOL_H

#include <atomic>
#include <thread>
#include <mutex>
#include <vector>
#include <list>
#include <functional>
#include <condition_variable>

namespace tp {

/** Usage:
    tp::ThreadPool<8> pool;
    for (int i = 0; i < 100; ++i)
    {
        pool.AddJob([]() {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        });
    }
    pool.JoinAll();
    std::cout << "All threads end" << std::endl;
*/

/**
 *  Simple ThreadPool that creates `ThreadCount` threads upon its creation,
 *  and pulls from a queue to get new jobs. The default is cpu core count threads.
 *
 *  This class requires a number of c++11 features be present in your compiler.
 */
class ThreadPool 
{
    
    std::vector<std::thread> threads;
    std::list<std::function<void(void)>> queue;

    std::atomic_int         jobs_left;
    std::atomic_bool        bailout;
    std::atomic_bool        finished;
    unsigned                threadCount;
    std::condition_variable job_available_var;
    std::condition_variable wait_var;
    std::mutex              wait_mutex;
    std::mutex              queue_mutex;

    /**
     *  Take the next job in the queue and run it.
     *  Notify the main thread that a job has completed.
     */
    void task() {
        while( !bailout ) {
            next_job()();
            --jobs_left;
            wait_var.notify_one();
        }
    }

    /**
     *  Get the next job; pop the first item in the queue, 
     *  otherwise wait for a signal from the main thread.
     */
    std::function<void(void)> next_job() {
        std::function<void(void)> res;
        std::unique_lock<std::mutex> job_lock( queue_mutex );

        // Wait for a job if we don't have any.
        job_available_var.wait( job_lock, [this]() ->bool { return queue.size() || bailout; } );
        
        // Get job from the queue
        if( !bailout ) {
            res = queue.front();
            queue.pop_front();
        }
        else { // If we're bailing out, 'inject' a job into the queue to keep jobs_left accurate.
            res = []{};
            ++jobs_left;
        }
        return res;
    }

public:
    ThreadPool(unsigned threadCount = 0)
    {
        jobs_left = 0;
        bailout = false;
        finished = false;

        if (threadCount == 0)
            threadCount = std::thread::hardware_concurrency();
        if (threadCount == 0)
            threadCount = 8;
        threads.reserve(threadCount);
        this->threadCount = threadCount;

        for( unsigned i = 0; i < threadCount; ++i )
			threads.push_back(std::thread([this] { this->task(); }));
    }

    /**
     *  JoinAll on deconstruction
     */
    ~ThreadPool() {
        joinAll();
    }

    /**
     *  Get the number of threads in this pool
     */
    inline unsigned size() const {
        return threadCount;
    }

    /**
     *  Get the number of jobs left in the queue.
     */
    inline unsigned remainingCount() {
        std::lock_guard<std::mutex> guard( queue_mutex );
        return queue.size();
    }

    /**
     *  Add a new job to the pool. If there are no jobs in the queue,
     *  a thread is woken up to take the job. If all threads are busy,
     *  the job is added to the end of the queue.
     */
    void addJob( std::function<void(void)> job ) {
        std::lock_guard<std::mutex> guard( queue_mutex );
        queue.emplace_back( job );
        ++jobs_left;
        job_available_var.notify_one();
    }

    /**
     *  Join with all threads. Block until all threads have completed.
     *  Params: WaitForAll: If true, will wait for the queue to empty 
     *          before joining with threads. If false, will complete
     *          current jobs, then inform the threads to exit.
     *  The queue will be empty after this call, and the threads will
     *  be done. After invoking `ThreadPool::JoinAll`, the pool can no
     *  longer be used. If you need the pool to exist past completion
     *  of jobs, look to use `ThreadPool::WaitAll`.
     */
    void joinAll( bool WaitForAll = true ) {
        if( !finished ) {
            if( WaitForAll ) {
                waitAll();
            }

            // note that we're done, and wake up any thread that's
            // waiting for a new job
            bailout = true;
            job_available_var.notify_all();

            for( auto &x : threads )
                if( x.joinable() )
                    x.join();
            finished = true;
        }
    }

    /**
     *  Wait for the pool to empty before continuing. 
     *  This does not call `std::thread::join`, it only waits until
     *  all jobs have finshed executing.
     */
    void waitAll() {
        if( jobs_left > 0 ) {
            std::unique_lock<std::mutex> lk( wait_mutex );
            wait_var.wait( lk, [this]{ return this->jobs_left == 0; } );
            lk.unlock();
        }
    }
};

} // namespace tp

#endif //TP_THREADPOOL_H
