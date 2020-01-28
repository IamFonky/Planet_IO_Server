#ifndef PLANETIO_SERVERS_H
#define PLANETIO_SERVERS_H

#include <boost/beast/core.hpp>
#include <boost/system/error_code.hpp>

#include <vector>

#include "../stellarPrimitives.h"

void socketError(boost::beast::error_code ec, char const* what);
void dataServer(std::string strAddress, unsigned short port,unsigned short nbThreads, Body* universe);
void eventServer(std::string strAddress, unsigned short port,unsigned short nbThreads, std::vector<Body>* blackholes);

#endif //PLANETIO_SERVERS_H
