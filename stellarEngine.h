#ifndef PLANETIO_STELLARENGINE_H
#define PLANETIO_STELLARENGINE_H

#include <map>
#include <vector>
#include "stellarPrimitives.h"
#include "engineConstants.h"

struct ServerParameters {
    bool running = true;
    bool displaying = false;
    bool paused = false;
    bool auto_respawn = true;
};

static unsigned long long last_event_id = 0;

struct Event {
    unsigned long long event_id;
    unsigned player_id;
    std::string message;
};

struct Events {
    unsigned long long last_event_id;
    Event *events;
};

void createEvent(Events *events, std::string message, unsigned player_id = 0);

typedef std::map<int, Blackhole> Blackholes;
typedef std::map<std::string,int> Players;

Blackhole createBlackhole(double x, double y, short level);

void createStellarBody(Body* body, unsigned playerId = 0);

void createStellarBodies(Body bodies[], bool deadBodies[], size_t max_bodies = NB_BODIES - NB_PLAYERS, bool alive = true);

void createWrecks(Body bodies[], bool deadBodies[], double* crash_mass, Vector *crash_position, Vector *crash_speed);

void fillEmptyWrecks(Body bodies[]);

struct Universe {
    ServerParameters* parameters;
    Body *bodies;
    bool *dead_bodies;
    Blackholes *blackholes;
    Events *events;
    Players *players;
    bool *used_slots;
};

void moveBodies(double dt, Universe *universe);

#endif //PLANETIO_STELLARENGINE_H
