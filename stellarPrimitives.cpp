#include <iostream>
#include <sstream>

#include "stellarPrimitives.h"
#include "stellarConstants.h"

using namespace std;

void displayVector(Vector vector) {
    cout << "x : " << vector.x << " , y : " << vector.y;
}

string stringify(Vector vector) {
    stringstream s;
    s << "{\"x\":" << vector.x << ",\"y\":" << vector.y << "}";
    return s.str();
}

void displayComposition(Composition composition) {
    cout << "gaz : " << composition.gaz << " , water : " << composition.water << " , earth : " << composition.earth;
}

string stringify(Composition composition) {
    stringstream s;
    s << "{\"gaz\":" << composition.gaz << ",\"water\":" << composition.water << ",\"earth\":" << composition.earth
      << "}";
    return s.str();
}

void displayBody(Body body) {
    displayBody(body, true);
}

void displayBody(Body body, bool displayDead) {
    if (displayDead || body.radius > 0) {
        cout << "Body of player : " << body.playerId;
        cout << " , position : (";
        displayVector(body.position);
        cout << ") , speed : (";
        displayVector(body.speed);
        cout << ") , composition : (";
        displayComposition(body.composition);
        cout << "), radius : " << body.radius << endl;
    }
}

string stringify(Body body) {
    stringstream s;
    s << "{\"position\":" << stringify(body.position) << ",\"speed\":" << stringify(body.speed) << ",\"composition\":"
      << stringify(body.composition) << ",\"radius\":" << body.radius << ",\"playerId\":" << body.playerId << "}";
    return s.str();
}

string stringify(Body bodies[], unsigned nbBodies) {
    stringstream s;
    s << "[";
    for (unsigned i = 0; i < nbBodies; ++i) {
        s << stringify(bodies[i]) << ((i != nbBodies - 1) ? "," : "");
    }
    s << "]";
    return s.str();
}
