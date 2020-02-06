////
////
//// Author : Pierre-Benjamin Monaco
////
//// Based on boost examples : https://github.com/boostorg/beast
////
//
//#include <boost/beast/core.hpp>
//#include <boost/beast/websocket.hpp>
//#include <boost/asio/dispatch.hpp>
//#include <boost/asio/strand.hpp>
//#include <boost/array.hpp>
//#include <algorithm>
//#include <cstdlib>
//#include <functional>
//#include <iostream>
//#include <memory>
//#include <string>
//#include <thread>
//#include <string>
//
//#include "servers.h"
//#include "server.hpp"
//#include "../stellarEngine.h"
//
//#include "document.h"     // rapidjson's DOM-style API
//#include "prettywriter.h" // for stringify JSON
//
//
//namespace beast = boost::beast;         // from <boost/beast.hpp>
//namespace http = beast::http;           // from <boost/beast/http.hpp>
//namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
//namespace net = boost::asio;            // from <boost/asio.hpp>
//using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
//
//// Echoes back all received WebSocket messages
//class EventSession : public Session<Universe, EventSession> {
//    unsigned long long last_read_event_id = 0;
//    unsigned player_id = 0;
//public:
//    // Take ownership of the socket
//    explicit
//    EventSession(tcp::socket &&eventSocket, Universe *blackholes) :
//            Session<Universe, EventSession>(std::move(eventSocket), blackholes) {
//
//    }
//
//    void
//    on_accept(beast::error_code ec) {
//        if (ec)
//            return socketError(ec, "accept");
//
////        do_read();
//
//        while(data_->parameters->running){
////            std::cout << "send event" << std::endl;
//            Event * event = &data_->events->events[last_read_event_id%NB_EVENTS];
//
//            if(last_event_id < data_->events->last_event_id
//               && event->player_id == 0 || event->player_id == player_id){
//                do_write_once(event->message);
//            }
//            usleep(1000000);
//        }
//
//    }
//
//
//    void
//    on_read(
//            beast::error_code ec,
//            std::size_t bytes_transferred) {
//        boost::ignore_unused(bytes_transferred);
//
//        // This indicates that the eventSession was closed
//        if (ec == websocket::error::closed)
//            return;
//        if (ec)
//            socketError(ec, "read");
//
//        // Echo the message
//        ws_.text(ws_.got_text());
//
//        // Clear the buffer
//        buffer_.consume(buffer_.size());
//
//
//        player_id = (unsigned) std::stoi(beast::buffers_to_string(buffer_.data()).c_str());
//        std::cout << "Player id : " << player_id << std::endl;
//    }
//
//
//};
//
//
//class EventListener : public Listener<Universe, EventListener, EventSession> {
//public:
//    EventListener(
//            net::io_context &ioc,
//            tcp::endpoint endpoint,
//            Universe *data) : Listener<Universe, EventListener, EventSession>(ioc, endpoint, data) {
//    }
//};
////------------------------------------------------------------------------------
//
//void eventServer(std::string strAddress, unsigned short port, unsigned short nbThreads, Universe *universe) {
//    server<Universe, EventListener>(strAddress, port, nbThreads, universe);
//}


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
// Example: WebSocket server, synchronous
//
//------------------------------------------------------------------------------

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>
#include <thread>

#include "servers.h"
#include "server.hpp"
#include "../stellarEngine.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

//------------------------------------------------------------------------------

// Echoes back all received WebSocket messages
void
do_session(tcp::socket &socket, Universe *universe) {
    unsigned player_id = 0;
    unsigned long long last_read_event_id = 0;

    // This buffer will hold the incoming message
    beast::flat_buffer buffer;

    try {
        // Construct the stream by moving in the socket
        websocket::stream<tcp::socket> ws{std::move(socket)};

        // Set a decorator to change the Server of the handshake
        ws.set_option(websocket::stream_base::decorator(
                [](websocket::response_type &res) {
                    res.set(http::field::server,
                            std::string(BOOST_BEAST_VERSION_STRING) +
                            " websocket-server-sync");
                }));

        // Accept the websocket handshake
        ws.accept();

        // Echo the message
        ws.text(ws.got_text());

        // Clear the buffer
        buffer.consume(buffer.size());

        // Reading first message
        ws.read(buffer);
        ws.text(ws.got_text());

        // Getting player id
        player_id = (unsigned) std::stoi(beast::buffers_to_string(buffer.data()).c_str());
        std::cout << "Player id : " << player_id << std::endl;


        while (universe->parameters->running) {
            Event *event = &universe->events->events[last_read_event_id % NB_EVENTS];


            if (last_read_event_id < universe->events->last_event_id) {
                if (event->player_id == 0 || event->player_id == player_id) {

                    ws.write(boost::asio::buffer(event->message, event->message.length()));

                    std::cout << "Send ";


                    }
                std::cout << "Event " << last_read_event_id << " : " << universe->events->last_event_id << " : " << event->message << std::endl;

                ++last_read_event_id;
                }


//            if(last_read_event_id < universe->events->last_event_id){
//                std::cout << "Event " << last_read_event_id << " : " << universe->events->last_event_id << " : " << event->message << std::endl;
//                ++last_read_event_id;
//            }

//            usleep(1000000);
            }
        }
        catch(beast::system_error const&se)
        {
            // This indicates that the session was closed
            if (se.code() != websocket::error::closed)
                std::cerr << "Error: " << se.code().message() << std::endl;
        }
        catch(std::exception const&e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

//------------------------------------------------------------------------------

    void eventServer(std::string strAddress, unsigned short port, unsigned short nbThreads, Universe *universe) {
        try {
            auto const address = net::ip::make_address(strAddress);

            // The io_context is required for all I/O
            net::io_context ioc{1};

            // The acceptor receives incoming connections
            tcp::acceptor acceptor{ioc, {address, port}};
            while (universe->parameters->running) {
                // This will receive the new connection
                tcp::socket socket{ioc};

                // Block until we get a connection
                acceptor.accept(socket);

                // Launch the session, transferring ownership of the socket
                std::thread{std::bind(
                        &do_session,
                        std::move(socket),
                        universe)}.detach();
            }
        }
        catch (const std::exception &e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
