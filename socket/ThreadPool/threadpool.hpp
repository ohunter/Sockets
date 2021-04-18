/*
This code is heavily based on the code from the following git repo:
https://github.com/progschj/ThreadPool

It has been modified in order to try and make it as much my own as the original
*/

#include <atomic>
#include <cstdint>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace Sockets {
    class ThreadPool {
        std::atomic_bool         state;
        std::vector<std::thread> workers;

        std::queue<std::function<void()>> jobs;
        std::mutex                        mtx;

        static void serve(std::atomic_bool &state, std::queue<std::function<void()>> &q,
                          std::mutex &mtx);

        public:
        ThreadPool(size_t N);

        ~ThreadPool();

        template <class F, class... Args>
        std::future<class std::result_of<F(Args...)>::type> schedule(F &&fn, Args &&... args) {
            using type = typename std::result_of<F(Args...)>::type;

            auto task = std::make_shared<std::packaged_task<type()>>(
                std::bind(std::forward<F>(fn, std::forward<Args>(args)...)));
            std::future<type> result = task->get_future();

            if (this->state.load()) {
                std::lock_guard<std::mutex> lock(this->mtx);
                this->jobs.emplace([task]() { (*task)(); });
            } else {
                throw std::runtime_error("Cannot schedule task for terminated threadpool");
            }

            return result;
        }

        template <class R, class... Args>
        std::future<class std::result_of<R(Args...)>::type> schedule(std::function<R(Args...)> fn) {
            auto           task   = std::make_shared<std::packaged_task<R()>>(fn);
            std::future<R> result = task->get_future();

            if (this->state.load()) {
                std::lock_guard<std::mutex> lock(this->mtx);
                this->jobs.emplace([task]() { (*task)(); });
            } else {
                throw std::runtime_error("Cannot schedule task for terminated threadpool");
            }

            return result;
        }
    };
} // namespace Sockets