#include "threadpool.hpp"

namespace Sockets {
    ThreadPool::ThreadPool(size_t N) {
        this->state.store(true);

        this->workers.reserve(N);
        for (size_t i = 0; i < N; i++)
            this->workers.emplace_back(this->serve, std::ref(this->state),
                                       std::ref(this->jobs),
                                       std::ref(this->mtx));
    }

    ThreadPool::~ThreadPool() {
        this->state.store(false);

        for (auto it = this->workers.begin(); it != this->workers.end(); it++)
            (*it).join();
    }

    void ThreadPool::serve(std::atomic_bool &state,
                           std::queue<std::function<void()>> &q,
                           std::mutex &mtx) {
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
} // namespace Sockets