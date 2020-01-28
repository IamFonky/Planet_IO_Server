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
#include <vector>
#include <string>

#include "servers.h"
#include "server.hpp"

#include "../stellarEngine.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

//------------------------------------------------------------------------------

/*
// Echoes back all received WebSocket messages
class dataSession : public std::enable_shared_from_this<dataSession>
{
    websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;
    Body* universe_;

public:
    // Take ownership of the socket
    explicit
    dataSession(tcp::socket&& socket,Body* universe)
        : ws_(std::move(socket))
        , universe_(universe)
    {
        ws_.binary(true);
        ws_.auto_fragment(true);
    }

    // Get on the correct executor
    void
    run()
    {
        // We need to be executing within a strand to perform async operations
        // on the I/O objects in this session. Although not strictly necessary
        // for single-threaded contexts, this example code is written to be
        // thread-safe by default.
        net::dispatch(ws_.get_executor(),
            beast::bind_front_handler(
                &dataSession::on_run,
                shared_from_this()));
    }

    // Start the asynchronous operation
    void
    on_run()
    {
        // Set suggested timeout settings for the websocket
        ws_.set_option(
            websocket::stream_base::timeout::suggested(
                beast::role_type::server));

        // Set a decorator to change the Server of the handshake
        ws_.set_option(websocket::stream_base::decorator(
            [](websocket::response_type& res)
            {
                res.set(http::field::server,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " websocket-server-async");
            }));
        // Accept the websocket handshake
        ws_.async_accept(
            beast::bind_front_handler(
                &dataSession::on_accept,
                shared_from_this()));
    }

    void
    on_accept(beast::error_code ec)
    {
        if(ec)
            return socketError(ec, "accept");

        // Read a message
        do_read();
    }

    void
    do_read()
    {
        double test = 1.0;

        // Read a message into our buffer
            ws_.async_read(
            buffer_,
            beast::bind_front_handler(
                &dataSession::on_read,
                shared_from_this()));
    }

    void
    on_read(
        beast::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        // This indicates that the session was closed
        if(ec == websocket::error::closed)
            return;

        if(ec)
            socketError(ec, "read");

        //std::cout << "on_read " << std::endl;

        // Echo the message
        ws_.text(ws_.got_text());

        //std::cout << "Data receive" << std::endl;
        //std::cout << beast::buffers_to_string(buffer_.data()) << std::endl;


        // Clear the buffer
        //buffer_.consume(buffer_.size());
        //beast::buffers(bodies);
        //beast::buffers_to_string(buffer_.data())

        std::string tosend = stringify(universe_,NB_BODIES);

        ws_.async_write(
            boost::asio::buffer(tosend, tosend.length()),
            beast::bind_front_handler(
                &dataSession::on_write,
                shared_from_this()));
    }

    void
    on_write(
        beast::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return socketError(ec, "write");

        // Clear the buffer
        buffer_.consume(buffer_.size());

        // Do another read
        do_read();
    }
};
*/

class DataSession : public Session<Body, DataSession>
{
public:
    // Take ownership of the socket
    explicit
    DataSession(tcp::socket&& eventSocket,Body* blackholes) :
            Session<Body, DataSession>(std::move(eventSocket) ,blackholes)
    {

    }

    void
    on_read(
            beast::error_code ec,
            std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        // This indicates that the session was closed
        if(ec == websocket::error::closed)
            return;

        if(ec)
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


        std::string tosend = stringify(data_,NB_BODIES);
        ws_.async_write(
                boost::asio::buffer(tosend, tosend.length()),
                beast::bind_front_handler(
                        &DataSession::on_write,
                        shared_from_this()));

        //do_write(stringify(data_,NB_BODIES));

    }

};

//------------------------------------------------------------------------------
/*
// Accepts incoming connections and launches the sessions
class dataListener : public std::enable_shared_from_this<dataListener>
{
    net::io_context& ioc_;
    tcp::acceptor acceptor_;
    Body* universe_;

public:
    dataListener(
        net::io_context& ioc,
        tcp::endpoint endpoint,
        Body* universe)
        : ioc_(ioc)
        , acceptor_(ioc)
        , universe_(universe)
    {
        beast::error_code ec;

        // Open the acceptor
        acceptor_.open(endpoint.protocol(), ec);
        if(ec)
        {
            socketError(ec, "open");
            return;
        }

        // Allow address reuse
        acceptor_.set_option(net::socket_base::reuse_address(true), ec);
        if(ec)
        {
            socketError(ec, "set_option");
            return;
        }

        // Bind to the server address
        acceptor_.bind(endpoint, ec);
        if(ec)
        {
            socketError(ec, "bind");
            return;
        }

        // Start listening for connections
        acceptor_.listen(
            net::socket_base::max_listen_connections, ec);
        if(ec)
        {
            socketError(ec, "listen");
            return;
        }
    }

    // Start accepting incoming connections
    void
    run()
    {
        do_accept();
    }

private:
    void
    do_accept()
    {
        // The new connection gets its own strand
        acceptor_.async_accept(
            net::make_strand(ioc_),
            beast::bind_front_handler(
                &dataListener::on_accept,
                shared_from_this()));
    }

    void
    on_accept(beast::error_code ec, tcp::socket socket)
    {
        if(ec)
        {
            socketError(ec, "accept");
        }
        else
        {
            // Create the session and run it
            std::make_shared<dataSession>(std::move(socket),universe_)->run();
        }

        // Accept another connection
        do_accept();
    }
};
 */

class DataListener : public Listener<Body, DataListener, DataSession>{
public:
    DataListener(
            net::io_context& ioc,
            tcp::endpoint endpoint,
            Body* data):Listener<Body, DataListener, DataSession>(ioc,endpoint,data){
    }
};

void dataServer(std::string strAddress, unsigned short port,unsigned short nbThreads, Body* universe){

    server<Body,DataListener>(strAddress,port, nbThreads, universe);

/*
    auto const address = net::ip::make_address(strAddress);
    auto const threads = std::max<int>(1, nbThreads);

    // The io_context is required for all I/O
    net::io_context ioc{threads};

    // Create and launch a listening port
    std::make_shared<dataListener>(ioc, tcp::endpoint{address, port}, universe)->run();

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for(auto i = threads - 1; i > 0; --i)
        v.emplace_back(
                [&ioc]
                {
                    ioc.run();
                });
    ioc.run();
*/
}