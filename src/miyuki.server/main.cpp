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

#include <ctime>
#include <iostream>
#include <string>
#include <asio.hpp>
#include <miyuki.renderer/graph.h>
#include "../core/export.h"

using asio::ip::tcp;
using namespace miyuki;

struct Message {
    size_t length;
    std::vector<char> data;

    explicit Message(const std::string &s) : length(s.length() + 1), data(s.length() + 1) {
        std::memcpy(data.data(), s.c_str(), s.length());
        data.back() = 0;
    }

    explicit Message(size_t N) : length(N), data(N) {}

    static Message read(tcp::socket &socket) {
        std::array<unsigned char, sizeof(size_t)> length{};
        asio::read(socket, asio::buffer(length));
        Message message(reinterpret_cast<size_t>(length.data()));
        asio::read(socket, asio::buffer(message.data));
        return message;
    }

    static void write(tcp::socket &socket, const Message &message) {
        std::array<unsigned char, sizeof(size_t)> length{};
        std::memcpy(reinterpret_cast<size_t *>(length.data()), &message.length, length.size());
        asio::write(socket, asio::buffer(length));
        asio::write(socket, asio::buffer(message.data));
    }
};

class RenderServer {
    bool _shutdown = false;
    std::shared_ptr<serialize::Context> ctx;
public:
    RenderServer() {
        ctx = core::Initialize();
    }

    bool isShutdown() const {
        return _shutdown;
    }

    void processMessage(const Message &message, tcp::socket &socket) {
        auto j = json::parse(message.data.begin(), message.data.end());
        if (j.is_object()) {
            auto act = j.at("action").get<std::string>();
            if (act == "render") {
                auto scene = serialize::fromJson<core::SceneGraph>(*ctx, j.at("scene"));
                scene.render(ctx, "out.png");
            }
        }
    }
};

int main() {

    try {
        asio::io_context io_context;

        std::unique_ptr<RenderServer> server(new RenderServer());

        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 13));

        while (!server->isShutdown()) {
            tcp::socket socket(io_context);
            acceptor.accept(socket);
            auto message = Message::read(socket);
            server->processMessage(message, socket);

            asio::error_code ignored_error;
            //asio::write(socket, asio::buffer(message), ignored_error);
        }
    }
    catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}