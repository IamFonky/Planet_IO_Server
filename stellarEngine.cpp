#include <cstdlib>
#include <math.h>
#include <iostream>
#include <float.h>

#include "stellarEngine.h"
#include "stellarPrimitives.h"
#include "stellarConstants.h"
#include "engineConstants.h"

using namespace std;

Blackhole createBlackhole(double x, double y, short level) {
    Blackhole blackhole;
    blackhole.position.x = x;
    blackhole.position.y = y;
    blackhole.mass = EARTH_MASS * 100 * level;

    blackhole.radius = 25.;

    return blackhole;
}

Body createStellarBody(unsigned playerId) {
    Body body;

    body.position.x = (double) rand() / (double) RAND_MAX * UNIVERSE_SIZE;
    body.position.y = (double) rand() / (double) RAND_MAX * UNIVERSE_SIZE;

    body.composition.gaz = (double) rand() / (double) RAND_MAX * EARTH_MASS / 30.0 + MOON_MASS;
    body.composition.water = (double) rand() / (double) RAND_MAX * EARTH_MASS / 30.0 + MOON_MASS;
    body.composition.earth = (double) rand() / (double) RAND_MAX * EARTH_MASS / 30.0 + MOON_MASS;

    double mass = body.composition.gaz + body.composition.water + body.composition.earth;
    body.speed.x = (double) rand() / (double) RAND_MAX * MOON_MASS / (mass * mass);
    body.speed.y = (double) rand() / (double) RAND_MAX * MOON_MASS / (mass * mass);

    body.radius = (double) rand() / (double) RAND_MAX * 1;
    body.playerId = playerId;

    return body;
}

void createStellarBodies(Body bodies[], bool deadBodies[], size_t max_bodies) {
    for (int i = 0; i < max_bodies; i++) {
        if (deadBodies[i]) {
            deadBodies[i] = false;
            bodies[i] = createStellarBody();
        }
    }
}


void moveBodies(double dt, Body bodies[], bool deadBodies[], Blackholes *blackholes) {
    for (int i = 0; i < NB_BODIES; ++i) {

        // Get data from body and initialize var
        Vector bodyPosition = bodies[i].position;
        Composition bodyComposition = bodies[i].composition;
        double bodyMass = bodyComposition.gaz + bodyComposition.water + bodyComposition.earth;
        double bodyVolume = bodyComposition.gaz / CompositonMV::gaz
                            + bodyComposition.water / CompositonMV::water
                            + bodyComposition.earth / CompositonMV::earth;

        double bodyRadius = cbrt(bodyVolume * 3.0 / 4.0 / (double) M_PI);

        Vector v_acceleration;
        v_acceleration.x = 0.0;
        v_acceleration.y = 0.0;
        bool collide = false;

        for (int j = 0; j < NB_BODIES; ++j) {
            // Get body values
            Vector body2Position = bodies[j].position;
            Composition body2Composition = bodies[j].composition;
            double body2Mass = body2Composition.gaz + body2Composition.water + body2Composition.earth;
            double body2Surface = body2Composition.gaz / CompositonMV::gaz
                                  + body2Composition.water / CompositonMV::water
                                  + body2Composition.earth / CompositonMV::earth;
            double body2Radius = cbrt(body2Surface * 3.0 / 4.0 / (double) M_PI);

            // Calculates distance between bodies
            Vector d_simple;
            d_simple.x = body2Position.x - bodyPosition.x;
            d_simple.y = body2Position.y - bodyPosition.y;
            // Calculates square distance between bodies
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
            collide |= (isDifferentBody && crash && lighter);

            // Add masses if collision occures and body is heavier
            bool eatBody2 = (isDifferentBody && crash && !lighter);
            bodyComposition.gaz += body2Composition.gaz * eatBody2;
            bodyComposition.water += body2Composition.water * eatBody2;
            bodyComposition.earth += body2Composition.earth * eatBody2;

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

//        std::map<int, Blackhole>::iterator blackhole;
//        if((blackhole = blackholes->find(bodies[i].playerId)) != blackholes->end()){
        std::map<int, Blackhole>::iterator blackhole = blackholes->begin();
        while (blackhole != blackholes->end()) {
            // Calculates distance between bodies
            Vector d_simple;
            d_simple.x = blackhole->second.position.x - bodyPosition.x;
            d_simple.y = blackhole->second.position.y - bodyPosition.y;
            // Calculates square distance between bodies
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


        // Set mass and radius to 0 if body got eaten by big one
        bodies[i].composition.gaz = bodyComposition.gaz * !collide + collide * 0.001;
        bodies[i].composition.water = bodyComposition.water * !collide + collide * 0.001;
        bodies[i].composition.earth = bodyComposition.earth * !collide + collide * 0.001;
        bodies[i].radius = bodyRadius * !collide;

        deadBodies[i] = collide;

        // Applies acceleration to speed on dt
        bodies[i].speed.x += v_acceleration.x * dt;
        bodies[i].speed.y += v_acceleration.y * dt;

        // Quick workadoung to avoid nan
        // TODO : FIND THE CAUSE OF NAN :P
        bodies[i].speed.x = isnan(bodies[i].speed.x) ? 0 : bodies[i].speed.x;
        bodies[i].speed.y = isnan(bodies[i].speed.y) ? 0 : bodies[i].speed.y;


        // Applies speed to position on dt
        bodies[i].position.x += bodies[i].speed.x * dt;
        bodies[i].position.y += bodies[i].speed.y * dt;



        // Gravity management on the edges to discuss
        // Make universe finite
        bool outRigth = bodies[i].position.x > UNIVERSE_SIZE;
        bool outLeft = bodies[i].position.x < 0;
        bool outBottom = bodies[i].position.y > UNIVERSE_SIZE;
        bool outTop = bodies[i].position.y < 0;

        bodies[i].position.x = 0.0 * (double) outRigth + UNIVERSE_SIZE * (double) outLeft +
                               bodies[i].position.x * (double) !(outRigth || outLeft);
        bodies[i].position.y = 0.0 * (double) outBottom + UNIVERSE_SIZE * (double) outTop +
                               bodies[i].position.y * (double) !(outRigth || outLeft);


    }
}

