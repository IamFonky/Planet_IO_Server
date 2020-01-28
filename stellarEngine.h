#ifndef PLANETIO_STELLARENGINE_H
#define PLANETIO_STELLARENGINE_H

#include <vector>
#include "stellarPrimitives.h"
#include "engineConstants.h"

void createStellarBodies(Body bs[], bool deadBodies[]);
void moveBodies(double dt, Body bs[], bool deadBodies[]);

#endif //PLANETIO_STELLARENGINE_H
