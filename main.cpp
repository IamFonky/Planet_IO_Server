#include <iostream>
#include <thread>
#include <chrono>
#include <ctime>
#include <unistd.h>

#include "stellarEngine.h"
#include "stellarPrimitives.h"
#include "stellarConstants.h"
#include "Websocket/servers.h"

using namespace std;

void bodiesEvents(Universe *universe) {
    auto tDeadBodies = chrono::system_clock::now();

    while (universe->parameters->running) {
        while (universe->parameters->paused);
        auto tEnd = chrono::system_clock::now();
        if (universe->parameters->auto_respawn && (tEnd - tDeadBodies).count() > 500000000.0) {
            createStellarBodies(universe->bodies, universe->dead_bodies);
            tDeadBodies = chrono::system_clock::now();
        }
    }
}

void bodiesEngine(Universe *universe) {
    auto tInit = chrono::system_clock::now();

    while (universe->parameters->running) {
        while (universe->parameters->paused);
        auto tEnd = chrono::system_clock::now();
        double dt = (tEnd - tInit).count() / 1000000000.0;
        moveBodies(dt, universe);
        tInit = tEnd;
        usleep(10000);
    }
}


void displayTask(Universe *universe) {
    while (universe->parameters->running) {
        while (universe->parameters->displaying) {
            while (universe->parameters->paused);
            for (int i = 0; i < NB_BODIES; ++i) {
                displayBody(universe->bodies[i]);
                cout << universe->dead_bodies[i] ? "DEAD" : "ALIVE";
            }
        }
    }
}

void controls(Universe *universe) {
    string arg;
    while (universe->parameters->running) {
        cout << "Commands : " << endl;
        cout << " OUTPUT - toggle show/hide logs" << endl;
        cout << " RESTART - restart the server" << endl;
        cout << " RESPAWN - respawn dead bodies" << endl;
        cout << " AUTO_RESPAWN - toggle auto-respawn every 5 seconds dead bodies" << endl;
        cout << " PAUSE - toggle pause/start the engine" << endl;
        cout << " QUIT - shut down the server" << endl;
        cin >> arg;
        if (arg.find("OUTPUT") != string::npos) {
            for (int i = 0; i < NB_BODIES; ++i) {
                displayBody(universe->bodies[i]);
            }
        }
        if (arg.find("RESTART") != string::npos) {
            universe->parameters->paused = true;
            std::fill_n(universe->dead_bodies, NB_BODIES, true);
            createStellarBodies(universe->bodies, universe->dead_bodies);

            universe->parameters->paused = false;
            cout << "RESTARTED" << endl;
        }
        if (arg.find("RESPAWN") != string::npos) {
            createStellarBodies(universe->bodies, universe->dead_bodies);
            cout << "RESPAWNED" << endl;
        }
        if (arg.find("AUTO_RESPAWN") != string::npos) {
            universe->parameters->auto_respawn != universe->parameters->auto_respawn;
            cout << "auto respawn : " << universe->parameters->auto_respawn << endl;
        }
        if (arg.find("PAUSE") != string::npos) {
            universe->parameters->paused != universe->parameters->paused;
            cout << (pause ? "PAUSED" : "STARTED") << endl;
        }
        if (arg.find("QUIT") != string::npos) {
            cout << "Just CTRL + C now!";
            universe->parameters->running = false;
        }
    }
}

int main() {
    Body bodies[NB_BODIES];
    bool dead_bodies[NB_BODIES];
    Blackholes blackholes;
    Event events[NB_EVENTS];
    Events events_state;
    events_state.last_event_id = last_event_id;
    events_state.events = events;
    Players players;
    ServerParameters parameters;
    bool used_slots[NB_PLAYERS];

    std::fill_n(dead_bodies, NB_BODIES, true);
    createStellarBodies(bodies, dead_bodies, NB_NON_WRECKS, false);
    fillEmptyWrecks(bodies);
    std::fill_n(used_slots,NB_PLAYERS,false);



    Universe universe;
    universe.bodies = bodies;
    universe.dead_bodies = dead_bodies;
    universe.blackholes = &blackholes;
    universe.events = &events_state;
    universe.players = &players;
    universe.parameters = &parameters;
    universe.used_slots = used_slots;


    for (int i = 0; i < NB_BODIES; ++i) {
        displayBody(universe.bodies[i]);
    }


    thread tUniverse(bodiesEngine, &universe);
    thread tUniverseEvents(bodiesEvents, &universe);
    thread tDisplay(displayTask, &universe);
    thread tControl(controls,&universe);
    thread tDataServer(dataServer, SERVER_IP, 28015, 1, &universe);
    thread tControlServer(controlServer, SERVER_IP, 28016, 1, &universe);
    thread tEventServer(eventServer, SERVER_IP, 28017, 1, &universe);

    while (parameters.running);

    tUniverse.detach();
    tUniverseEvents.detach();
    tDisplay.detach();
    tControl.detach();
    tDataServer.detach();
    tControlServer.detach();
    tEventServer.detach();

    return 0;
}