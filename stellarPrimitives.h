#ifndef PLANETIO_STELLARPRIMITIVES_H
#define PLANETIO_STELLARPRIMITIVES_H

#include <string>


class Vector {
public:
    double x;
    double y;
};

void displayVector(Vector vector);

std::string stringify(Vector vector);

class Composition {
public:
    double gaz;
    double water;
    double earth;
};

struct CompositonMV : public Composition {
public:
    static constexpr double gaz = 0.1;
    static constexpr double water = 1.0;
    static constexpr double earth = 10.0;
};

void displayComposition(Composition composition);

std::string stringify(Composition composition);

class Body {
public:
    Vector position;
    Vector speed;
    Composition composition;
    double radius;
    unsigned playerId;
};

void displayBody(Body body);

void displayBody(Body body, bool displayDead);

std::string stringify(Body body);

std::string stringify(Body bodies[], unsigned nbBodies);

class Blackhole {
public:
    Vector position;
    double mass;
    double radius;
    unsigned playerId;
};


#endif //PLANETIO_STELLARPRIMITIVES_H
