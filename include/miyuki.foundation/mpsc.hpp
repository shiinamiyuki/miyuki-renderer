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

#ifndef MIYUKIRENDERER_MPSC_HPP
#define MIYUKIRENDERER_MPSC_HPP

#include <thread>
#include <mutex>
#include <deque>
#include <condition_variable>
#include <chrono>
#include <optional>
#include "noncopyable.hpp"

namespace miyuki::mpsc {
    namespace detail {
        template<class T>
        struct Receiver;

        template<class T>
        struct Sender;

        template<class T>
        struct Channel {
            std::deque<T> deque;
            std::mutex mutex;
            std::condition_variable cv;
            std::weak_ptr<Sender<T>> sender;
            std::weak_ptr<Receiver<T>> receiver;

            template<class... Args>
            bool send(Args &&... args) {
                if (receiver.expired()) {
                    return false;
                }
                std::unique_lock<std::mutex> lock(mutex);
                deque.emplace_back(std::forward<T>(args)...);
                cv.notify_one();
                return true;
            }

            [[nodiscard]] bool isEmpty() const {
                return deque.empty();
            }

            std::optional<T> recv() {
                if (sender.expired() && isEmpty()) {
                    return {};
                }
                std::unique_lock<std::mutex> lock(mutex);
                cv.wait(lock, [=]() {
                    return !isEmpty();
                });
                auto value = std::move(deque.front());
                deque.pop_front();
                return value;
            }

        };

        template<class T>
        struct Sender {


            std::shared_ptr<Channel<T>> channel;
        };

        template<class T>
        struct Receiver {


            std::shared_ptr<Channel<T>> channel;
        };
    }

    template<class T>
    struct Sender;
    template<class T>
    struct Receiver;

    template<class T>
    struct channel_t {
        Sender<T> tx;
        Receiver<T> rx;
    };


    template<class T>
    channel_t<T> channel();


    template<class T>
    struct Sender {

        friend struct detail::Sender<T>;

        friend channel_t<T> channel<T>();

        template<class... Args>
        bool send(Args &&...args) const {
            return sender->channel->send(std::forward<Args>(args)...);
        }

        void close()const { sender = nullptr; }

    private:
        Sender() = default;


        mutable std::shared_ptr<detail::Sender<T>> sender;
    };

    template<class T>
    struct Receiver {
        friend struct detail::Receiver<T>;

        friend channel_t<T> channel<T>();

        std::optional<T> recv() const {
            return receiver->channel->recv();
        }

        [[nodiscard]] bool block()const{
            return receiver->channel->isEmpty();
        }
    private:


        std::shared_ptr<detail::Receiver<T>> receiver;

        Receiver() = default;
    };


    template<class T>
    channel_t<T> channel() {
        auto channel = std::make_shared<detail::Channel<T>>();
        auto sender = std::make_shared<detail::Sender<T>>();
        auto receiver = std::make_shared<detail::Receiver<T>>();
        channel->receiver = receiver;
        channel->sender = sender;

        sender->channel = channel;
        receiver->channel = channel;

        Sender<T> tx;
        tx.sender = sender;

        Receiver<T> rx;
        rx.receiver = receiver;

        channel_t<T> ch{tx, rx};
        return std::move(ch);
    }
}


#endif //MIYUKIRENDERER_MPSC_HPP
