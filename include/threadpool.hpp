/* 
This code is heavily based on the code from the following git repo:
https://github.com/progschj/ThreadPool

It has been modified in order to try and make it as much my own as the original
*/


#include <cstdint>
#include <queue>
#include <mutex>
#include <thread>
#include <future>
#include <atomic>
#include <functional>
#include <vector>

namespace Sockets {
        class ThreadPool {
                std::atomic_bool state;
                std::vector<std::thread> workers;

                std::queue<std::function<void()>> jobs;

                std::mutex c_lock; // Consumer lock
                std::mutex p_lock; // Producer lock

                static void serve(std::atomic_bool& state, std::queue<std::function<void()>>& q, std::mutex& mtx)
                {
                        while (state.load()) {
                                std::function<void()> task;

                                {
                                        const std::lock_guard<std::mutex> lock(mtx);

                                        if (!q.empty()) {
                                                task = q.front();
                                                q.pop();
                                        }
                                }

                                if (task)
                                        task();
                        }
                }

                public:
                ThreadPool(size_t N)
                {
                        this->state.store(true);

                        this->workers.reserve(N);
                        for (size_t i = 0; i < N; i++)
                                this->workers.emplace_back(this->serve, std::ref(this->state), std::ref(this->jobs), std::ref(this->c_lock));
                }

                ~ThreadPool()
                {
                        this->state.store(false);

                        for (auto it = this->workers.begin(); it != this->workers.end(); it++)
                                (*it).join();
                }


                template<class F, class... Args>
                std::future<class std::result_of<F(Args...)>::type> schedule(F&& fn, Args&&... args)
                {
                        using type = typename std::result_of<F(Args...)>::type;

                        auto task = std::make_shared<std::packaged_task<type()>>(std::bind(std::forward<F>(fn, std::forward<Args>(args)...)));
                        std::future<type> result = task->get_future();

                        if (this->state.load()) {
                                std::lock_guard<std::mutex> lock(this->p_lock);
                                this->jobs.emplace([task]() {(*task)();});
                        }
                        else {
                                throw std::runtime_error("Cannot schedule task for terminated threadpool");
                        }

                        return result;
                }
        };

        // template class ThreadPool<unsigned int>;
        // 
}