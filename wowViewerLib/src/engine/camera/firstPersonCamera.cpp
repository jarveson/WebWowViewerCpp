//
// Created by deamon on 18.05.17.
//

#include "firstPersonCamera.h"
#include "math.h"

void FirstPersonCamera::addForwardDiff(float val) {
    this->depthDiff = this->depthDiff + val;
}

void FirstPersonCamera::addHorizontalViewDir(float val) {
    this->ah = ah + this->ah;
}
void FirstPersonCamera::addVerticalViewDir(float val) {
    float av = this->av;
    av += val;

    if (av < -89.99999f) {
        av = -89.99999f;
    } else if (av > 89.99999f) {
        av = 89.99999f;
    }
    this->av = av;
}

void FirstPersonCamera::startMovingForward(){
    this->MDDepthPlus = 1;
}
void FirstPersonCamera::stopMovingForward(){
    this->MDDepthPlus = 0;
}
void FirstPersonCamera::startMovingBackwards(){
    this->MDDepthMinus = 1;
}
void FirstPersonCamera::stopMovingBackwards(){
    this->MDDepthMinus = 0;
}

void FirstPersonCamera::startStrafingLeft(){
    this->MDHorizontalMinus = 1;
}
void FirstPersonCamera::stopStrafingLeft(){
    this->MDHorizontalMinus = 0;
}
void FirstPersonCamera::startStrafingRight(){
    this->MDHorizontalPlus = 1;
}
void FirstPersonCamera::stopStrafingRight(){
    this->MDHorizontalPlus = 0;
}

void FirstPersonCamera::startMovingUp(){
    this->MDVerticalPlus = 1;
}
void FirstPersonCamera::stopMovingUp(){
    this->MDVerticalPlus = 0;
}
void FirstPersonCamera::startMovingDown(){
    this->MDVerticalMinus = 1;
}
void FirstPersonCamera::stopMovingDown(){
    this->MDVerticalMinus = 0;
}

mathfu::vec3 FirstPersonCamera::getCameraPosition(){
    return camera;
}
mathfu::vec3 FirstPersonCamera::getCameraLookAt(){
    return lookAt;
}


void FirstPersonCamera::tick (float timeDelta) {
    mathfu::vec3 dir = {1, 0, 0};
    float moveSpeed = 0.02;
    mathfu::vec3 camera = this->camera;

    float dTime = timeDelta;

    float horizontalDiff = dTime * moveSpeed * (this->MDHorizontalPlus - this->MDHorizontalMinus);
    float depthDiff      = dTime * moveSpeed * (this->MDDepthPlus - this->MDDepthMinus) + this->depthDiff;
    float verticalDiff   = dTime * moveSpeed * (this->MDVerticalPlus - this->MDVerticalMinus);

    this->depthDiff = 0;

    /* Calc look at position */


    dir = mathfu::mat3::RotationY(this->av/M_PI_2) * dir;
    dir = mathfu::mat3::RotationZ(-this->ah/M_PI_2) * dir;
    dir = mathfu::normalize(dir);

    /* Calc camera position */
    if (horizontalDiff != 0) {
        mathfu::vec3 right = mathfu::mat3::RotationZ(-90/M_PI_2) * dir;
        right[2] = 0;

        right = mathfu::normalize(right);
        right = right *horizontalDiff;

        camera = camera + right;
    }

    if (depthDiff != 0) {
        mathfu::vec3 movDir = dir;

        movDir = movDir * depthDiff;
        camera = camera + movDir;
    }
    if (verticalDiff != 0) {
        camera[2] = camera[2] + verticalDiff;
    }

    this->lookAt = camera + dir;
}
void FirstPersonCamera :: setCameraPos (float x, float y, float z) {
    //Reset camera
    this->camera[0] = x;
    this->camera[1] = y;
    this->camera[2] = z;

    this->lookAt[0] = 0;
    this->lookAt[1] = 0;
    this->lookAt[2] = 0;

    this->av = 0;
    this->ah = 0;
}