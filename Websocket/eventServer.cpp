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

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

typedef std::vector<Body> Data;

// Echoes back all received WebSocket messages
class EventSession : public Session<Data, EventSession>
{
public:
    // Take ownership of the socket
    explicit
    EventSession(tcp::socket&& eventSocket,Data* blackholes) :
            Session<Data, EventSession>(std::move(eventSocket) ,blackholes)
    {

    }

    void
    on_read(
        beast::error_code ec,
        std::size_t bytes_transferred)
    {

        boost::ignore_unused(bytes_transferred);

        // This indicates that the eventSession was closed
        if(ec == websocket::error::closed)
            return;

        if(ec)
            socketError(ec, "read");

        // Echo the message
        ws_.text(ws_.got_text());


        std::cout << "Event receive" << std::endl;
        //const char* test;
        //std::sscanf(beast::buffers_to_string(buffer_.data()),&test,"{message:%s");
        std::cout << beast::buffers_to_string(buffer_.data()) << std::endl;

        // Clear the buffer
        //buffer_.consume(buffer_.size());
        //beast::buffers(bodies);
        //beast::buffers_to_string(buffer_.data())

        //std::string tosend = stringify(blackholes_,NB_BODIES);

        do_write("1234");
    }
};


class EventListener : public Listener<Data, EventListener, EventSession>{
public:
    EventListener(
            net::io_context& ioc,
    tcp::endpoint endpoint,
            Data* data):Listener<Data, EventListener, EventSession>(ioc,endpoint,data){
    }
};
//------------------------------------------------------------------------------

void eventServer(std::string strAddress, unsigned short port,unsigned short nbThreads, Data* events){
    server<Data,EventListener>(strAddress,port, nbThreads, events);
}
