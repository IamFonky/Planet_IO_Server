//
//
// Author : Pierre-Benjamin Monaco
//
// Based on boost examples : https://github.com/boostorg/beast
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
#include <string>

#include "servers.h"
#include "server.hpp"

#include "../stellarEngine.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

class DataSession : public Session<Universe, DataSession> {
public:
    // Take ownership of the socket
    explicit
    DataSession(tcp::socket &&eventSocket, Universe *universe) :
            Session<Universe, DataSession>(std::move(eventSocket), universe) {

    }

    void
    on_read(
            beast::error_code ec,
            std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        // This indicates that the session was closed
        if (ec == websocket::error::closed)
            return;

        // If error, display
        if (ec)
            socketError(ec, "read");

        // Echo the message
        ws_.text(ws_.got_text());

        // Clear the buffer
        buffer_.consume(buffer_.size());

        // Send bodies
        do_write(stringify(data_->bodies, NB_BODIES));

    }

};

class DataListener : public Listener<Universe, DataListener, DataSession> {
public:
    DataListener(
            net::io_context &ioc,
            tcp::endpoint endpoint,
            Universe *data) : Listener<Universe, DataListener, DataSession>(ioc, endpoint, data) {
    }
};

void dataServer(std::string strAddress, unsigned short port, unsigned short nbThreads, Universe *universe) {

    server<Universe, DataListener>(strAddress, port, nbThreads, universe);
}