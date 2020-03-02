// MIT License
// 
// Copyright (c) 2019 椎名深雪
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef MIYUKIRENDERER_TASK_HPP
#define MIYUKIRENDERER_TASK_HPP

#include <type_traits>
#include <atomic>
#include <future>
#include <optional>
#include "noncopyable.hpp"

namespace miyuki {
    template<class T>
    class Task {
    public:
        using Opt = std::optional<T>;
        using ContFunc = std::function<bool(void)>;
        using TaskFunc = std::function<Opt(const ContFunc &)>;
    private:
        TaskFunc f;
        enum State {
            Stopped,
            Running,
            Suspended,
            Killed,
            Finished
        };
        std::atomic<State> state = Stopped;
        std::future<Opt> future;
    public:

        Task() = default;

        explicit Task(const TaskFunc &f) : f(f) {}

        Task(Task &&task) noexcept : f(std::move(task.f)), future(std::move(task.future)), state(State(task.state)) {}

        Task &operator=(Task &&task) noexcept {
            f = std::move(task.f);
            future = std::move(task.future);
            state = State(task.state);
            return *this;
        }

        void launch() {
            state = Running;
            auto func = std::move(f);
            f = TaskFunc();
            future = std::async(std::launch::async, func, [=]() -> bool {
                while (state == Suspended) {}
                return state == Running;
            });
        }

        void kill() {
            state = Killed;
        }

        void suspend() {
            state = Suspended;
        }

        Opt wait() {
            auto opt = future.get();
            state = Finished;
            return std::move(opt);

        }
    };
}
#endif //MIYUKIRENDERER_TASK_HPP
