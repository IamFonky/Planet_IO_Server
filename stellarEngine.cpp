#include <cstdlib>
#include <cmath>
#include <iostream>
#include <cfloat>

#include "stellarEngine.h"
#include "stellarPrimitives.h"
#include "stellarConstants.h"
#include "engineConstants.h"

using namespace std;

void createEvent(Events *events, std::string message, unsigned player_id) {
    events->last_event_id = last_event_id;
    events->events[last_event_id % NB_EVENTS].message = message;
    events->events[last_event_id % NB_EVENTS].player_id = player_id;
    events->events[last_event_id % NB_EVENTS].event_id = last_event_id++;
}

Blackhole createBlackhole(double x, double y, short level) {
    Blackhole blackhole;
    blackhole.position.x = x;
    blackhole.position.y = y;
    blackhole.mass = EARTH_MASS * 100 * level;
    blackhole.radius = 25.;
    return blackhole;
}

void createStellarBody(Body *body, unsigned playerId) {
    body->position.x = (double) rand() / (double) RAND_MAX * UNIVERSE_SIZE;
    body->position.y = (double) rand() / (double) RAND_MAX * UNIVERSE_SIZE;

    body->composition.gaz = (double) rand() / (double) RAND_MAX * EARTH_MASS / 30.0 + MOON_MASS;
    body->composition.water = (double) rand() / (double) RAND_MAX * EARTH_MASS / 30.0 + MOON_MASS;
    body->composition.earth = (double) rand() / (double) RAND_MAX * EARTH_MASS / 30.0 + MOON_MASS;

    double mass = body->composition.gaz + body->composition.water + body->composition.earth;
    body->speed.x = (double) rand() / (double) RAND_MAX * MOON_MASS / (mass * mass);
    body->speed.y = (double) rand() / (double) RAND_MAX * MOON_MASS / (mass * mass);

    body->radius = (double) rand() / (double) RAND_MAX * 1;
    body->playerId = playerId;
}

void createStellarBodies(Body bodies[], bool deadBodies[], size_t max_bodies, bool alive) {
    for (int i = 0; i < max_bodies; i++) {
        if (deadBodies[i]) {
            deadBodies[i] = !alive;
            createStellarBody(&bodies[i]);
        }
    }
}

void createWrecks(Body bodies[], bool deadBodies[], double *crash_mass, Vector *crash_position, Vector *crash_speed) {
    double mass = *crash_mass;
    auto nb_wrecks = (short) round(rand() / (double) RAND_MAX * 7 + 3);
    double wreck_mass;
    Vector position;

    short proportions[10];
    short total_proportions = 0;
    for (size_t i = 0; i < nb_wrecks; ++i) {
        proportions[i] = (short) (rand() / (double) RAND_MAX * 100) + (short)1;
        total_proportions += proportions[i];
    }

    Composition composition;
    composition.gaz = rand() / (double) RAND_MAX;
    composition.water = rand() / (double) RAND_MAX;
    composition.earth = rand() / (double) RAND_MAX;
//    cout << "composition" << endl;
//    displayComposition(composition);
//    cout << endl;
//
    double total_compositions = composition.gaz + composition.water + composition.earth;
    Composition composition_prop;
    composition_prop.gaz = composition.gaz / total_compositions;
    composition_prop.earth = composition.earth / total_compositions;
    composition_prop.water = composition.water / total_compositions;
//    cout << "composition_prop" << endl;
//    displayComposition(composition_prop);
//    cout << endl;


    double total_volume = mass * composition_prop.gaz / CompositonMV::gaz
                          + mass * composition_prop.water / CompositonMV::water
                          + mass * composition_prop.earth / CompositonMV::earth;
    double total_radius = cbrt(total_volume * 3.0 / 4.0 / (double) M_PI);

    double inertia = /*mass * */ sqrt(crash_speed->x * crash_speed->x + crash_speed->y * crash_speed->y);
    cout << "======================================" << endl;
    cout << "BOOOM "<< nb_wrecks <<endl;
    cout << "CRASH POSITION " << endl;
    displayVector(*crash_position);
    cout << endl;

    for (int i = NB_NON_WRECKS; i < NB_BODIES && nb_wrecks > 0; i++) {
        if (deadBodies[i]) {
            deadBodies[i] = false;

            wreck_mass = mass * (double) proportions[nb_wrecks - 1] / (double) total_proportions;

            bodies[i].composition.gaz = wreck_mass * composition_prop.gaz;
            bodies[i].composition.water = wreck_mass * composition_prop.water;
            bodies[i].composition.earth = wreck_mass * composition_prop.earth;

            position.x = total_radius * (rand() / (double) RAND_MAX * 2. - 1. ) + DBL_MIN;
            position.y = sqrt(total_radius * total_radius - position.x * position.x) * round(rand() / RAND_MAX * 2 - 1) + DBL_MIN;

            cout << "RELATIVE POSITION " << endl;
            displayVector(position);
            cout << endl;

            bodies[i].position.x = position.x + crash_position->x;
            bodies[i].position.y = position.y + crash_position->y;

            cout << "ABSOLUTE POSITION " << endl;
            displayVector(bodies[i].position);
            cout << endl;

            bodies[i].speed.x = inertia / wreck_mass * position.x / (position.x + position.y);
            bodies[i].speed.y = inertia / wreck_mass * position.y / (position.x + position.y);

            bodies[i].radius = 0;
            bodies[i].playerId = 0;

            cout << "MASS " << mass << endl;
            cout << "WRECK MASS " << wreck_mass << endl;
            cout << "SPEED" << endl;
            displayVector(bodies[i].speed);
            cout << endl;


            --nb_wrecks;
        }
    }
    cout << "======================================" << endl;
}

void fillEmptyWrecks(Body bodies[]) {
    for (size_t i = NB_NON_WRECKS; i < NB_BODIES; ++i) {
        bodies[i].playerId = 0;
        bodies[i].radius = 0.;
        bodies[i].position.x = 0.;
        bodies[i].position.y = 0.;
        bodies[i].composition.gaz = 0.;
        bodies[i].composition.water = 0.;
        bodies[i].composition.earth = 0.;
        bodies[i].speed.x = 0.;
        bodies[i].speed.y = 0.;
    }
}

void moveBodies(double dt, Universe *universe) {
    for (int i = 0; i < NB_BODIES; ++i) {

        // Get data from body and initialize var
        Vector bodyPosition = universe->bodies[i].position;
        Vector bodySpeed = universe->bodies[i].speed;
        Composition bodyComposition = universe->bodies[i].composition;
        double bodyMass = bodyComposition.gaz + bodyComposition.water + bodyComposition.earth;
        double bodyVolume = bodyComposition.gaz / CompositonMV::gaz
                            + bodyComposition.water / CompositonMV::water
                            + bodyComposition.earth / CompositonMV::earth;

        double bodyRadius = cbrt(bodyVolume * 3.0 / 4.0 / (double) M_PI);

        Vector v_acceleration;
        v_acceleration.x = 0.0;
        v_acceleration.y = 0.0;
        Vector crash_position;
        crash_position.x = 0.0;
        crash_position.y = 0.0;
        Vector crash_speed;
        crash_speed.x = 0.0;
        crash_speed.y = 0.0;
        double crashed_mass = 0.0;

        bool collide = false;
        bool explode = false;

        for (int j = 0; j < NB_BODIES; ++j) {
            // Get body values
            Vector body2Position = universe->bodies[j].position;
            Vector body2Speed = universe->bodies[j].speed;
            Composition body2Composition = universe->bodies[j].composition;
            double body2Mass = body2Composition.gaz + body2Composition.water + body2Composition.earth;
            double body2Surface = body2Composition.gaz / CompositonMV::gaz
                                  + body2Composition.water / CompositonMV::water
                                  + body2Composition.earth / CompositonMV::earth;
            double body2Radius = cbrt(body2Surface * 3.0 / 4.0 / (double) M_PI);

            // Calculates distance between universe->bodies
            Vector d_simple;
            d_simple.x = body2Position.x - bodyPosition.x;
            d_simple.y = body2Position.y - bodyPosition.y;
            // Calculates square distance between universe->bodies
            Vector d_square;
            d_square.x = d_simple.x * d_simple.x;
            d_square.y = d_simple.y * d_simple.y;

            // Check if body is the same
            bool isDifferentBody = i != j;
            double d_square_sum = d_square.x + d_square.y;

            // Avoid division by zero
            d_square_sum += !isDifferentBody;

            // Calculates distance
            double distance = sqrt(d_square_sum
                                   + DBL_MIN //avoiding NaN
            );


            // Checks for collisions
            bool crash = ((distance - bodyRadius - body2Radius) <= 0.0);
            bool lighter = (bodyMass < body2Mass);
            bool critically_lighter = std::abs(bodyMass - body2Mass) < (bodyMass + body2Mass) * CRITICAL_RATIO;
            bool collide_now = (isDifferentBody && crash && lighter);
            bool explode_now = (isDifferentBody && crash && critically_lighter);
            bool is_wreck = i >= WRECKS_INDEX;
            collide |= collide_now;
            explode |= explode_now && !is_wreck;

            crash_position.x += (bodyPosition.x + body2Position.x) / 2.0 * explode_now;
            crash_position.y += (bodyPosition.y + body2Position.y) / 2.0 * explode_now;
            crash_speed.x += (sqrt(bodySpeed.x * bodySpeed.x) + sqrt(body2Speed.x * body2Speed.x)) * explode_now;
            crash_speed.y += (sqrt(bodySpeed.y * bodySpeed.y) + sqrt(body2Speed.y * body2Speed.y)) * explode_now;
            crashed_mass += (bodyMass + body2Mass) * explode_now;

            // Add masses if collision occures and body is heavier
            bool eatBody2 = (isDifferentBody && crash && !lighter);
            universe->bodies[i].composition.gaz += body2Composition.gaz * eatBody2;
            universe->bodies[i].composition.water += body2Composition.water * eatBody2;
            universe->bodies[i].composition.earth += body2Composition.earth * eatBody2;

            universe->bodies[j].composition.gaz = body2Composition.gaz * !eatBody2;
            universe->bodies[j].composition.water = body2Composition.water * !eatBody2;
            universe->bodies[j].composition.earth = body2Composition.earth * !eatBody2;

            // Calculates acceleration
            double acceleration = body2Mass / d_square_sum;

            // Avoid computing same planet gravitation (means infinite acceleration)
            acceleration = isDifferentBody * !(isDifferentBody && crash && lighter) * acceleration;

            // Avoid mooving sun (as the sun (first body) is the first body in the array)
            //acceleration *= (bool) bodyIndex;

            // Set mass to 0 if collision occures with bigger body
            //bodyMass *= !(isDifferentBody && crash && lighter);

            // Calculates acceleration factor and apply it to the 3 dimensions
            double a_factor = acceleration / distance;
            v_acceleration.x += a_factor * d_simple.x;
            v_acceleration.y += a_factor * d_simple.y;
        }



        /////////////////////////////////////////////////////////////////////////////////////
        // INTERACTION WITH BLACKHOLE
        /////////////////////////////////////////////////////////////////////////////////////

        std::map<int, Blackhole>::iterator blackhole;
        if ((blackhole = universe->blackholes->find(universe->bodies[i].playerId)) != universe->blackholes->end()) {
//        std::map<int, Blackhole>::iterator blackhole = universe->blackholes->begin();
//        while (blackhole != universe->blackholes->end()) {
            // Calculates distance between universe->bodies
            Vector d_simple;
            d_simple.x = blackhole->second.position.x - bodyPosition.x;
            d_simple.y = blackhole->second.position.y - bodyPosition.y;
            // Calculates square distance between universe->bodies
            Vector d_square;
            d_square.x = d_simple.x * d_simple.x;
            d_square.y = d_simple.y * d_simple.y;
            double d_square_sum = d_square.x + d_square.y;

            // Calculates distance
            double distance = sqrt(d_square_sum);
            bool crash = ((distance - bodyRadius - blackhole->second.radius) <= 0.0);


            // Calculates acceleration
            double acceleration = !crash * blackhole->second.mass / d_square_sum;

            // Avoid mooving sun (as the sun (first body) is the first body in the array)
            //acceleration *= (bool) bodyIndex;

            // Set mass to 0 if collision occures with bigger body
            //bodyMass *= !(isDifferentBody && crash && lighter);

            // Calculates acceleration factor and apply it to the 3 dimensions
            double a_factor = acceleration / distance;
            v_acceleration.x += a_factor * d_simple.x;
            v_acceleration.y += a_factor * d_simple.y;

            ++blackhole;
        }


        /////////////////////////////////////////////////////////////////////////////////////
        // END INTERACTION WITH BLACKHOLE
        /////////////////////////////////////////////////////////////////////////////////////

//        bool died = (collide || explode);
        bool died = collide;

        // Set mass and radius to 0 if body got eaten by big one
//        universe->bodies[i].composition.gaz = universe->bodies[i].composition.gaz * !died + died * 0.001;
//        universe->bodies[i].composition.water = universe->bodies[i].composition.gaz * !died + died * 0.001;
//        universe->bodies[i].composition.earth = universe->bodies[i].composition.gaz * !died + died * 0.001;
        universe->bodies[i].radius = bodyRadius * !died;

        universe->dead_bodies[i] |= died;

        if (explode) {
            createWrecks(universe->bodies, universe->dead_bodies, &crashed_mass, &crash_position, &crash_speed);
        }
        if (collide) {
            if (universe->bodies[i].playerId > 0) {
                createEvent(universe->events,
                            "{\"message\" : \"PLAYER_DIED\" , \"id\" : " + to_string(universe->bodies[i].playerId) +
                            "}",
                            universe->bodies[i].playerId);
            }
        }

        // Applies acceleration to speed on dt
        universe->bodies[i].speed.x += v_acceleration.x * dt;
        universe->bodies[i].speed.y += v_acceleration.y * dt;

        // Quick workadoung to avoid nan
        // TODO : FIND THE CAUSE OF NAN :P
        universe->bodies[i].speed.x = isnan(universe->bodies[i].speed.x) ? 0 : universe->bodies[i].speed.x;
        universe->bodies[i].speed.y = isnan(universe->bodies[i].speed.y) ? 0 : universe->bodies[i].speed.y;


        // Applies speed to position on dt
        universe->bodies[i].position.x += universe->bodies[i].speed.x * dt;
        universe->bodies[i].position.y += universe->bodies[i].speed.y * dt;



        // Gravity management on the edges to discuss
        // Make universe finite
        bool outRigth = universe->bodies[i].position.x > UNIVERSE_SIZE;
        bool outLeft = universe->bodies[i].position.x < 0;
        bool outBottom = universe->bodies[i].position.y > UNIVERSE_SIZE;
        bool outTop = universe->bodies[i].position.y < 0;

        universe->bodies[i].position.x = 0.0 * (double) outRigth + UNIVERSE_SIZE * (double) outLeft +
                                         universe->bodies[i].position.x * (double) !(outRigth || outLeft);
        universe->bodies[i].position.y = 0.0 * (double) outBottom + UNIVERSE_SIZE * (double) outTop +
                                         universe->bodies[i].position.y * (double) !(outRigth || outLeft);


    }
}
