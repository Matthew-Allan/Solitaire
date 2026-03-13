#include "ship.h"

#include <stddef.h>

void applyVelocity(ShipData *ship) {
    vec3Add(ship->trans.offset, ship->velocity, ship->trans.offset, 1);
}

void applyForce(ShipData *ship, const vec3 force) {
    vec3Add(ship->velocity, force, ship->velocity, 1);
}

void shipThrust(ShipData *ship, const vec3 direction, float magnitude) {
    vec3 force;
    quatMltVec(ship->trans.orientation, direction, force);
    vec3Norm(force, force);
    vec3MltSlr(force, magnitude, force, 1);
    applyForce(ship, force);
}

void shipRotate(ShipData *ship, const vec3 angles) {
    quaternion rotation;
    quatFromEuler(rotation, angles[vecX], angles[vecY], angles[vecZ]);
    quatMlt(ship->trans.orientation, rotation, ship->trans.orientation);
}

static const vec3 *thrust_dirs[] = {
    &FORWARD_VEC,
    &BACKWARD_VEC,
    &LEFT_VEC,
    &RIGHT_VEC,
    &UP_VEC,
    &DOWN_VEC
};

static const vec3 *rotate_dirs[] = {
    &X_AXIS,
    &Y_AXIS,
    &Z_AXIS
};
#include <stdio.h>

void handleInput(ShipData *ship, SHIP_Flags flags, uint64_t delta_time) {
    float thrust_speed = delta_time / 10000.f;
    float rotate_speed = delta_time / 1000.f;

    for(int i = 0; i < 6; i++) {
        if(FLAG_INDEX_SET(flags, i)) {
            shipThrust(ship, varr(thrust_dirs[i]), thrust_speed);
        }
    }
    vec3 rotation;
    for(int i = 6; i < 12; i++) {
        if (FLAG_INDEX_SET(flags, i)) {
            vec3MltSlr(varr(rotate_dirs[i / 2 - 3]), rotate_speed * (((i % 2) * 2) - 1), rotation, 1);
            shipRotate(ship, rotation);
        }
    }
}

void initShip(ShipData *ship, const vec3 pos) {
    initTrans(&ship->trans, pos, NULL);
    setVec3(ship->velocity, 0, 0, 0);
}