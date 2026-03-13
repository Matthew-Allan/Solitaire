#ifndef SHIP_H
#define SHIP_H

#include <seng/transform.h>

#define FLAG_MODULE_NAME SHIP

#define FLAG_LIST \
    FLAG_ELEMENT(FORWARD) \
    FLAG_ELEMENT(BACKWARD) \
    FLAG_ELEMENT(LEFT) \
    FLAG_ELEMENT(RIGHT) \
    FLAG_ELEMENT(UP) \
    FLAG_ELEMENT(DOWN) \
    FLAG_ELEMENT(PITCH_DOWN) \
    FLAG_ELEMENT(PITCH_UP) \
    FLAG_ELEMENT(YAW_RIGHT) \
    FLAG_ELEMENT(YAW_LEFT) \
    FLAG_ELEMENT(ROLL_RIGHT) \
    FLAG_ELEMENT(ROLL_LEFT)

#include <seng/flags.h>

typedef struct ShipData {
    Transform trans;
    vec3 velocity;
} ShipData;

void applyVelocity(ShipData *ship);

void applyForce(ShipData *ship, const vec3 force);

void shipThrust(ShipData *ship, const vec3 direction, float magnitude);

void handleInput(ShipData *ship, SHIP_Flags flags, uint64_t delta_time);

void initShip(ShipData *ship, const vec3 pos);

#endif