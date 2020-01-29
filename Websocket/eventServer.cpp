//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: WebSocket server, asynchronous
//
//------------------------------------------------------------------------------

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/array.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <string>

#include "servers.h"
#include "server.hpp"
#include "../stellarEngine.h"

#include "document.h"     // rapidjson's DOM-style API
#include "prettywriter.h" // for stringify JSON


namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

// Echoes back all received WebSocket messages
class EventSession : public Session<Universe, EventSession> {
    int player_id = 0;
public:
    // Take ownership of the socket
    explicit
    EventSession(tcp::socket &&eventSocket, Universe *blackholes) :
            Session<Universe, EventSession>(std::move(eventSocket), blackholes) {

    }

    void
    on_read(
            beast::error_code ec,
            std::size_t bytes_transferred) {

        boost::ignore_unused(bytes_transferred);

        // This indicates that the eventSession was closed
        if (ec == websocket::error::closed)
            return;

        if (ec)
            socketError(ec, "read");

        // Echo the message
        ws_.text(ws_.got_text());


        rapidjson::Document document;  // Default template parameter uses UTF8 and MemoryPoolAllocator.
        // "normal" parsing, decode strings to new buffers. Can use other input stream via ParseStream().
        if (document.Parse(beast::buffers_to_string(buffer_.data()).c_str()).HasParseError()) {
            std::cerr << "Parsing event failed" << std::endl;

        } else {
            //std::string strmessageType = document["message"].GetString();
            int messageType = document["message"].GetInt();
            switch (messageType) {
                case 0: // CREATE BLACKHOLE
                case 1: // MOVE BLACKHOLE
//                    std::cout << document["position"]["x"].GetDouble()
//                              << " "
//                              << document["position"]["y"].GetDouble()
//                              << std::endl;
//                    if(player_id != 0){
                    (*data_->blackholes)[player_id] = createBlackhole(
                            document["position"]["x"].GetDouble(),
                            document["position"]["y"].GetDouble(),
                            (short) document["level"].GetInt()
                    );
//                    }
                    break;
                case 2: // KILL BLACKHOLE
                    data_->blackholes->erase(player_id);
                    break;
                case 3: // NEW PLAYER

                    do_write("{\"message\" : \"PLAYER_ACCEPTED\"}");
                    return;
            }

        }

        buffer_.consume(buffer_.size());

        do_read();

    }
};


class EventListener : public Listener<Universe, EventListener, EventSession> {
public:
    EventListener(
            net::io_context &ioc,
            tcp::endpoint endpoint,
            Universe *data) : Listener<Universe, EventListener, EventSession>(ioc, endpoint, data) {
    }
};
//------------------------------------------------------------------------------

void eventServer(std::string strAddress, unsigned short port, unsigned short nbThreads, Universe *universe) {
    server<Universe, EventListener>(strAddress, port, nbThreads, universe);
}
