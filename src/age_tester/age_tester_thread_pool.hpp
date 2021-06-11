//
// Copyright 2021 Christoph Sprenger
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#ifndef AGE_TESTER_THREAD_POOL_HPP
#define AGE_TESTER_THREAD_POOL_HPP

#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>



namespace age::tester
{
    template<typename Element>
    class blocking_vector
    {
    public:
        void push(Element t)
        {
            std::unique_lock lock(m_mutex);
            m_vector.emplace_back(std::move(t));
        }

        std::vector<Element> copy() const
        {
            std::unique_lock lock(m_mutex);
            return m_vector;
        }

        size_t size() const
        {
            std::unique_lock lock(m_mutex);
            return m_vector.size();
        }

    private:
        mutable std::mutex   m_mutex; //!< mutable to allow locking in const functions
        std::vector<Element> m_vector;
    };



    using task_t = std::function<void(void)>;

    class thread_pool
    {
        AGE_DISABLE_COPY(thread_pool);

    public:
        explicit thread_pool(size_t num_threads = std::thread::hardware_concurrency())
        {
            for (size_t i = 0; i < num_threads; ++i)
            {
                m_threads.emplace_back(std::thread([this]() {
                    while (true)
                    {
                        task_t task = nullptr;
                        {
                            std::unique_lock lock(m_mutex);
                            m_cv.wait(lock,
                                      [this]() {
                                          return !m_tasks.empty() || should_terminate();
                                      });

                            // no tasks left, all threads idle => terminate?
                            if (should_terminate())
                            {
                                break;
                            }

                            // check for next task while locked
                            if (!m_tasks.empty())
                            {
                                task = m_tasks.back();
                                m_tasks.pop_back();
                                if (task != nullptr)
                                {
                                    ++m_working_threads_count;
                                }
                            }
                        }

                        // run task while not locked
                        if (task != nullptr)
                        {
                            task();
                            {
                                std::unique_lock lock(m_mutex);
                                --m_working_threads_count; // possible should_terminate() change => notify all other threads
                            }
                            m_cv.notify_all();
                        }
                    }
                }));
            }
        }

        ~thread_pool() noexcept
        {
            // trigger thread termination
            {
                std::unique_lock lock(m_mutex);
                m_terminate_when_idle = true; // possible should_terminate() change => notify all threads
            }
            m_cv.notify_all();

            // wait for all threads to terminate
            for (auto& thread : m_threads)
            {
                thread.join();
            }
        }

        void queue_task(task_t task)
        {
            {
                std::unique_lock lock(m_mutex);
                m_tasks.emplace_back(std::move(task));
            }
            m_cv.notify_one();
        }

    private:
        std::vector<std::thread> m_threads;

        std::mutex              m_mutex;
        std::condition_variable m_cv;
        std::vector<task_t>     m_tasks;
        bool                    m_terminate_when_idle   = false;
        int                     m_working_threads_count = 0;

        [[nodiscard]] bool should_terminate() const
        {
            return m_tasks.empty() && m_terminate_when_idle && (m_working_threads_count == 0);
        }
    };

} // namespace age::tester



#endif // AGE_TESTER_THREAD_POOL_HPP
