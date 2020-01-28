//
// Created by imfonky on 1/26/20.
//

#ifndef PLANETIO_SERVER_H
#define PLANETIO_SERVER_H

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

//
// Modifications Authors :
//  - Pierre-Benjamin Monaco
//


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

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

//------------------------------------------------------------------------------


// Echoes back all received WebSocket messages
template <class T, class S>
class Session : public std::enable_shared_from_this<S>
{
protected:
    websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;
    T* data_;

public:
    explicit
    // Take ownership of the socket
    Session(tcp::socket&& socket,T* data)
            : ws_(std::move(socket))
            , data_(data)
    {
        ws_.binary(true);
        ws_.auto_fragment(true);
        }

    // Get on the correct executor
    void
    run()
    {
        // We need to be executing within a strand to perform async operations
        // on the I/O objects in this Session. Although not strictly necessary
        // for single-threaded contexts, this example code is written to be
        // thread-safe by default.
        net::dispatch(ws_.get_executor(),
                      beast::bind_front_handler(
                              &S::on_run,
                              std::enable_shared_from_this<S>::shared_from_this()));
    }

    // Start the asynchronous operation
    void
    on_run()
    {
        // Set suggested timeout settings for the websocket
        ws_.set_option(
                websocket::stream_base::timeout::suggested(
                        beast::role_type::server));
        // Avoiding timout on websockets

        //        websocket::stream_base::timeout opt{
//                std::chrono::seconds(30),   // handshake timeout
//                //websocket::stream_base::none(),        // idle timeout
//                std::chrono::seconds(60),
//                false
//        };


        // Set a decorator to change the Server of the handshake
        ws_.set_option(websocket::stream_base::decorator(
                [](websocket::response_type& res)
                {
                    res.set(http::field::server,
                            std::string(BOOST_BEAST_VERSION_STRING) +
                            " websocket-server-async");
                }));


        // Setting messages size
        ws_.read_message_max(65536);
        ws_.write_buffer_bytes(UINT_MAX);


        // Accept the websocket handshake
        ws_.async_accept(
                beast::bind_front_handler(
                        &S::on_accept,
                        std::enable_shared_from_this<S>::shared_from_this()));
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
                        &S::on_read,
                        std::enable_shared_from_this<S>::shared_from_this()));
    }

    void
    on_read(
            beast::error_code ec,
            std::size_t bytes_transferred)
    {
        std::cout << "Basic on_read" << std::endl;

        boost::ignore_unused(bytes_transferred);

        // This indicates that the Session was closed
        if(ec == websocket::error::closed)
            return;

        if(ec)
            socketError(ec, "read");

    }

    void do_write(std::string to_send){
        buffer_.consume(buffer_.size());
        ws_.async_write(
                boost::asio::buffer(to_send, to_send.length()),
                beast::bind_front_handler(
                        &S::on_write,
                        std::enable_shared_from_this<S>::shared_from_this()));
    }

    void
    on_write(
            beast::error_code ec,
            std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);



        if(ec == websocket::error::closed)
            return;
        if(ec)
            return socketError(ec, "write");



        do_read();
    }

    void
    on_close(beast::error_code ec)
    {
        std::cout << "Closing socket"<< std::endl;
        if(ec)
            return socketError(ec, "close");

        // If we get here then the connection is closed gracefully

        // The make_printable() function helps print a ConstBufferSequence
        std::cout << beast::make_printable(buffer_.data()) << std::endl;
    }
};


//------------------------------------------------------------------------------

// Accepts incoming connections and launches the Sessions
template <class T, class L,  class S>
class Listener : public std::enable_shared_from_this<L>
{
protected:
    net::io_context& ioc_;
    tcp::acceptor acceptor_;
    T* data_;

public:
    Listener(
            net::io_context& ioc,
            tcp::endpoint endpoint,
            T* data)
            : ioc_(ioc)
            , acceptor_(ioc)
            , data_(data)
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

protected:
    void
    do_accept()
    {
        // The new connection gets its own strand
        acceptor_.async_accept(
                net::make_strand(ioc_),
                beast::bind_front_handler(
                        &L::on_accept,
                        std::enable_shared_from_this<L>::shared_from_this()));
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
            // Create the Session and run it
            std::make_shared<S>(std::move(socket),data_)->run();
        }

        // Accept another connection
        do_accept();
    }

};

template <class T,class L>
void server(std::string strAddress, unsigned short port,unsigned short nbThreads, T* data){

    auto const address = net::ip::make_address(strAddress);
    auto const threads = std::max<int>(1, nbThreads);

    // The io_context is required for all I/O
    net::io_context ioc{threads};

    // Create and launch a listening port
    std::make_shared<L>(ioc, tcp::endpoint{address, port}, data)->run();

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
}




#endif //PLANETIO_SERVER_H
