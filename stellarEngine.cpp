#include <cstdlib>
#include <math.h>
#include <iostream>

#include "stellarEngine.h"
#include "stellarPrimitives.h"
#include "stellarConstants.h"
#include "engineConstants.h"

using namespace std;


void createStellarBodies(Body bodies[], bool deadBodies[]) {
    for (int i = 0; i < NB_BODIES; i++) {
        if(deadBodies[i]){
            deadBodies[i] = false;

            bodies[i].position.x = (double)rand() / (double)RAND_MAX * D_SUN_EARTH;
            bodies[i].position.y = (double)rand() / (double)RAND_MAX * D_SUN_EARTH;

            bodies[i].composition.gaz = (double)rand() / (double)RAND_MAX * EARTH_MASS / 30.0 + MOON_MASS;
            bodies[i].composition.water = (double)rand() / (double)RAND_MAX * EARTH_MASS / 30.0 + MOON_MASS;
            bodies[i].composition.earth = (double)rand() / (double)RAND_MAX * EARTH_MASS / 30.0 + MOON_MASS;

            double mass = bodies[i].composition.gaz + bodies[i].composition.water + bodies[i].composition.earth;
            bodies[i].speed.x = (double)rand() / (double)RAND_MAX * MOON_MASS / (mass * mass);
            bodies[i].speed.y = (double)rand() / (double)RAND_MAX * MOON_MASS / (mass * mass);

            bodies[i].radius = (double)rand() / (double)RAND_MAX * 1;
            bodies[i].playerId = 0;
        }
    }
}

void moveBodies(double dt, Body bodies[], bool deadBodies[]){
    for(int i = 0; i < NB_BODIES; ++i) {

        // Get data from body and initialize var
        Vector bodyPosition = bodies[i].position;
        Composition bodyComposition = bodies[i].composition;
        double bodyMass = bodyComposition.gaz + bodyComposition.water + bodyComposition.earth;
        double bodySurface = bodyComposition.gaz / CompositonMV::gaz
                            + bodyComposition.water / CompositonMV::water
                            + bodyComposition.earth / CompositonMV::earth;

        double bodyRadius = cbrt(bodySurface * 3.0 / 4.0 / (double)M_PI);

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
            double body2Radius = cbrt(body2Surface * 3.0 / 4.0 / (double)M_PI);

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
            double distance = sqrt(d_square_sum);


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

        // Set mass and radius to 0 if body got eaten by big one
        bodies[i].composition.gaz = bodyComposition.gaz * !collide + collide * 0.001;
        bodies[i].composition.water = bodyComposition.water * !collide + collide * 0.001;
        bodies[i].composition.earth = bodyComposition.earth * !collide + collide * 0.001;
        bodies[i].radius = bodyRadius * !collide;

        deadBodies[i] = collide;

        // Applies acceleration to speed on dt
        bodies[i].speed.x += v_acceleration.x * dt;
        bodies[i].speed.y += v_acceleration.y * dt;

        // Applies speed to position on dt
        bodies[i].position.x += bodies[i].speed.x * dt;
        bodies[i].position.y += bodies[i].speed.y * dt;

        // Make universe infinite
        bool outRigth = bodies[i].position.x > D_SUN_EARTH;
        bool outLeft = bodies[i].position.x < 0;
        bool outBottom = bodies[i].position.y > D_SUN_EARTH;
        bool outTop = bodies[i].position.y < 0;

        // Make universe finite
        bodies[i].position.x = 0.0 * (double) outRigth + D_SUN_EARTH * (double) outLeft +
                               bodies[i].position.x * (double) !(outRigth || outLeft);
        bodies[i].position.y = 0.0 * (double) outBottom + D_SUN_EARTH * (double) outTop +
                               bodies[i].position.y * (double) !(outRigth || outLeft);

    }
}
