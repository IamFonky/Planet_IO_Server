#ifndef PLANETIO_SERVERS_H
#define PLANETIO_SERVERS_H

#include <boost/beast/core.hpp>
#include <boost/system/error_code.hpp>

#include <vector>

#include "../stellarEngine.h"

void socketError(boost::beast::error_code ec, char const *what);

void dataServer(std::string strAddress, unsigned short port, unsigned short nbThreads, Body *bodies);

void eventServer(std::string strAddress, unsigned short port, unsigned short nbThreads, Universe *universe);

#endif //PLANETIO_SERVERS_H
