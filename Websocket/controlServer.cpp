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

#include "document.h"     // rapidjson's DOM-style API
#include "prettywriter.h" // for stringify JSON


namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

// Echoes back all received WebSocket messages
class ControlSession : public Session<Universe, ControlSession> {
    bool usedSlots[NB_PLAYERS];

    size_t findEmptySlot(){
        for(int i = 0 ; i < NB_PLAYERS; ++i){
            if(!usedSlots[i]){
                return i;
            }
        }
        return -1;
    }

public:
    // Take ownership of the socket
    explicit
    ControlSession(tcp::socket &&eventSocket, Universe *universe) :
            Session<Universe, ControlSession>(std::move(eventSocket), universe) {
        std::fill_n(usedSlots,NB_PLAYERS,true);
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
//            std::cout << messageType << std::endl;
            switch (messageType) {
                case 0: // CREATE BLACKHOLE
                case 1: // MOVE BLACKHOLE
                    (*data_->blackholes)[session_id] = createBlackhole(
                            document["position"]["x"].GetDouble(),
                            document["position"]["y"].GetDouble(),
                            (short) document["level"].GetInt()
                    );
//                    }
                    break;
                case 2: // KILL BLACKHOLE
                    data_->blackholes->erase(session_id);
                    break;
                case 3: // NEW PLAYER
                    std::string player_name = document["name"].GetString();

                    auto inserted = data_->players->insert(std::pair<std::string, int>(player_name, session_id));

                    if(inserted.second) {
                        size_t free_slot = findEmptySlot();
                        if (free_slot >= 0) {
                            usedSlots[free_slot] = false;
                            createStellarBody(&(data_->bodies[free_slot + PLAYER_INDEX]),session_id);
                            do_write(
                                    R"({"message" : "PLAYER_ACCEPTED", "id" : )" + std::to_string(session_id) + R"( , "name" : ")" + player_name + R"(" })");
                        } else {
                            do_write("{\"message\" : \"PLAYER_REFUSED\" , \"reason\" : \"NO_SLOT\" }");
                        }
                    } else {
                        do_write(R"({"message" : "PLAYER_REFUSED" , "reason" : "EXISTS" })");
                    }
                    return;
//                case 4: // PLAYER LEAVE
//
//                    break;

            }

        }

        buffer_.consume(buffer_.size());

        do_read();

    }
};


class Controlistener : public Listener<Universe, Controlistener, ControlSession> {
public:
    Controlistener(
            net::io_context &ioc,
            tcp::endpoint endpoint,
            Universe *data) : Listener<Universe, Controlistener, ControlSession>(ioc, endpoint, data) {
    }
};
//------------------------------------------------------------------------------

void controlServer(std::string strAddress, unsigned short port, unsigned short nbThreads, Universe *universe) {
    server<Universe, Controlistener>(strAddress, port, nbThreads, universe);
}
