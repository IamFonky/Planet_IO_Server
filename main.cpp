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

bool running = true;
bool displaying = false;
bool paused = false;
bool autoRespawn = true;


void bodiesEvents(Body bodies[], bool deadBodies[]) {
    auto tDeadBodies = chrono::system_clock::now();

    while (running) {
        while (paused);
        auto tEnd = chrono::system_clock::now();
        if (autoRespawn && (tEnd - tDeadBodies).count() > 500000000.0) {
            createStellarBodies(bodies, deadBodies);
            tDeadBodies = chrono::system_clock::now();
        }
    }
}

void bodiesEngine(Body *bodies, bool deadBodies[], Blackholes *blackholes) {
    auto tInit = chrono::system_clock::now();

    while (running /*&& test < 2*/) {
        while (paused);
        auto tEnd = chrono::system_clock::now();
        double dt = (tEnd - tInit).count() / 1000000000.0;
        moveBodies(dt, bodies, deadBodies, blackholes);
        tInit = tEnd;
    }
}


void displayTask(Body *bodies, bool deadBodies[]) {
    while (running) {
        while (displaying) {
            while (paused);
            for (int i = 0; i < NB_BODIES; ++i) {
                displayBody(bodies[i]);
                cout << deadBodies[i] ? "DEAD" : "ALIVE";
            }
        }
    }
}

void controls(Body bodies[], bool deadBodies[]) {
    string arg;
    while (running) {
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
                displayBody(bodies[i]);
                cout << deadBodies[i] ? "DEAD" : "ALIVE";
            }
        }
        if (arg.find("RESTART") != string::npos) {
            paused = true;
            std::fill_n(deadBodies, NB_BODIES, true);
            createStellarBodies(bodies, deadBodies);
            paused = false;
            cout << "RESTARTED" << endl;
        }
        if (arg.find("RESPAWN") != string::npos) {
            createStellarBodies(bodies, deadBodies);
            cout << "RESPAWNED" << endl;
        }
        if (arg.find("AUTO_RESPAWN") != string::npos) {
            autoRespawn != autoRespawn;
            cout << "auto respawn : " << autoRespawn << endl;
        }
        if (arg.find("PAUSE") != string::npos) {
            paused != paused;
            cout << (pause ? "PAUSED" : "STARTED") << endl;
        }
        if (arg.find("QUIT") != string::npos) {
            cout << "Just CTRL + C now!";
            running = false;
        }

        //Periodic actions
        if (displaying) {
            for (int i = 0; i < NB_BODIES; ++i) {
                displayBody(bodies[i]);
                cout << deadBodies[i] ? "DEAD" : "ALIVE";
            }
        }
    }
}

int main() {
    Body bodies[NB_BODIES];
    bool deadBodies[NB_BODIES];
    Blackholes blackholes;

    Universe universe;
    universe.blackholes = &blackholes;
    universe.bodies = bodies;

    std::fill_n(deadBodies, NB_BODIES, true);
    createStellarBodies(bodies, deadBodies, NB_BODIES);

    thread tUniverse(bodiesEngine, bodies, deadBodies, &blackholes);
    thread tUniverseEvents(bodiesEvents, bodies, deadBodies);
    thread tDisplay(displayTask, bodies, deadBodies);
    thread tControl(controls, bodies, deadBodies);
    thread tDataServer(dataServer, "127.0.0.1", 28015, 1, bodies);
    thread tEventServer(eventServer, "127.0.0.1", 28016, 1, &universe);

    while (running);

    tUniverse.detach();
    tUniverseEvents.detach();
    tDisplay.detach();
    tControl.detach();
    tDataServer.detach();
    tEventServer.detach();

    return 0;
}