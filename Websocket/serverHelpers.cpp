//
// Created by imfonky on 1/27/20.
//

#include <iostream>

#include "servers.h"

// Report a socket error
void
socketError(boost::beast::error_code ec, char const *what) {
    std::cerr << what << ": " << ec.message() << "\n";
}
