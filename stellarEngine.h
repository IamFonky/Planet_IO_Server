#ifndef PLANETIO_STELLARENGINE_H
#define PLANETIO_STELLARENGINE_H

#include <map>
#include <vector>
#include "stellarPrimitives.h"
#include "engineConstants.h"

typedef std::map<int, Blackhole> Blackholes;
typedef std::map<int, std::string> Players;

struct Universe {
    Blackholes *blackholes;
    Body *bodies;
};

Blackhole createBlackhole(double x, double y, short level);

Body createStellarBody(unsigned playerId = 0);

void createStellarBodies(Body bs[], bool deadBodies[], size_t max_bodies = NB_BODIES - NB_PLAYERS);

void moveBodies(double dt, Body bs[], bool deadBodies[], Blackholes *blackholes);

#endif //PLANETIO_STELLARENGINE_H
