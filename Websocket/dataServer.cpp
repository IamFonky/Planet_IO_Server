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

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

class DataSession : public Session<Body, DataSession> {
public:
    // Take ownership of the socket
    explicit
    DataSession(tcp::socket &&eventSocket, Body *blackholes) :
            Session<Body, DataSession>(std::move(eventSocket), blackholes) {

    }

    void
    on_read(
            beast::error_code ec,
            std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        // This indicates that the session was closed
        if (ec == websocket::error::closed)
            return;

        if (ec)
            socketError(ec, "read");

        //std::cout << "on_read " << std::endl;

        // Echo the message
        ws_.text(ws_.got_text());

        //std::cout << "Data receive" << std::endl;
        //std::cout << beast::buffers_to_string(buffer_.data()) << std::endl;


        // Clear the buffer
        buffer_.consume(buffer_.size());
        //beast::buffers(bodies);
        //beast::buffers_to_string(buffer_.data())


//        std::string tosend = stringify(data_,NB_BODIES);
//        ws_.async_write(
//                boost::asio::buffer(tosend, tosend.length()),
//                beast::bind_front_handler(
//                        &DataSession::on_write,
//                        shared_from_this()));

        do_write(stringify(data_, NB_BODIES));

    }

};

class DataListener : public Listener<Body, DataListener, DataSession> {
public:
    DataListener(
            net::io_context &ioc,
            tcp::endpoint endpoint,
            Body *data) : Listener<Body, DataListener, DataSession>(ioc, endpoint, data) {
    }
};

void dataServer(std::string strAddress, unsigned short port, unsigned short nbThreads, Body *universe) {

    server<Body, DataListener>(strAddress, port, nbThreads, universe);
}