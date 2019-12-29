//
// main.cpp
// ~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <chrono>
#include <cstdio>
#include <httplib.h>
#include <miyuki.renderer/graph.h>
#include <miyuki.foundation/log.hpp>
#include "../core/export.h"

#define SERVER_CERT_FILE "./cert.pem"
#define SERVER_PRIVATE_KEY_FILE "./key.pem"


namespace miyuki::server {
    using namespace httplib;

    std::string dump_headers(const Headers &headers) {
        std::string s;
        char buf[BUFSIZ];

        for (auto it = headers.begin(); it != headers.end(); ++it) {
            const auto &x = *it;
            snprintf(buf, sizeof(buf), "%s: %s\n", x.first.c_str(), x.second.c_str());
            s += buf;
        }

        return s;
    }

    std::string log(const Request &req, const Response &res) {
        std::string s;
        char buf[BUFSIZ];

        s += "================================\n";

        snprintf(buf, sizeof(buf), "%s %s %s", req.method.c_str(),
                 req.version.c_str(), req.path.c_str());
        s += buf;

        std::string query;
        for (auto it = req.params.begin(); it != req.params.end(); ++it) {
            const auto &x = *it;
            snprintf(buf, sizeof(buf), "%c%s=%s",
                     (it == req.params.begin()) ? '?' : '&', x.first.c_str(),
                     x.second.c_str());
            query += buf;
        }
        snprintf(buf, sizeof(buf), "%s\n", query.c_str());
        s += buf;

        s += dump_headers(req.headers);

        s += "--------------------------------\n";

        snprintf(buf, sizeof(buf), "%d %s\n", res.status, res.version.c_str());
        s += buf;
        s += dump_headers(res.headers);
        s += "\n";

        if (!res.body.empty()) { s += res.body; }

        s += "\n";

        return s;
    }

    class RenderServer {
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        SSLServer svr(SERVER_CERT_FILE, SERVER_PRIVATE_KEY_FILE);
#else
        Server svr;
#endif
        int port;
        std::optional<core::SceneGraph> graph;
        std::shared_ptr<serialize::Context> context;
    public:
        explicit RenderServer(int port) : port(port) {
            if (!svr.is_valid()) {
                fprintf(stderr, "server has an error...\n");
                exit(1);
            }
            context = core::Initialize();
            svr.Post("/render/start", [=](const Request &req, Response &res, const ContentReader &content_reader) {
                auto it = req.headers.find("Content-Type");
                if (it != req.headers.end()) {
                    auto type = it->second;
                    if (type == "application/json") {
                        std::string body;
                        content_reader([&](const char *data, size_t data_length) {
                            body.append(data, data_length);
                            return true;
                        });
                        auto data = json::parse(body);
                        auto path = data.at("workdir").get<std::string>();
                        res.set_content(body, "text/plain");
                        fs::current_path(fs::absolute(fs::path(path)));
                        auto &scene = data.at("scene");
                        graph = serialize::fromJson<core::SceneGraph>(*context, scene);
                        graph->render(context, "out.png");

                    }
                }
            });
            svr.Get("/render/status", [=](const Request &req, Response &res) {
                auto body = R"({ "status":"stopped" })";
                res.set_content(body, "application/json");
            });

            svr.Get("/render/output", [=](const Request &req, Response &res) {
                auto body = R"({ "status":"stopped" })";
                res.set_content(body, "application/json");
            });

            svr.Get("/stop",
                    [&](const Request & /*req*/, Response & /*res*/) { svr.stop(); });

            svr.set_error_handler([](const Request & /*req*/, Response &res) {
                const char *fmt = "<p>Error Status: <span style='color:red;'>%d</span></p>";
                char buf[BUFSIZ];
                snprintf(buf, sizeof(buf), fmt, res.status);
                res.set_content(buf, "text/html");
            });

            svr.set_logger([](const Request &req, const Response &res) {
                // printf("%s", log(req, res).c_str());
            });

        }

        void run() {
            svr.listen("localhost", port);
        }
    };
}

int main(int argc, char **argv) {

    miyuki::server::RenderServer server(8080);
    server.run();
    return 0;
}
