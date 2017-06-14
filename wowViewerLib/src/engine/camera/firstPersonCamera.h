//
// Created by deamon on 18.05.17.
//

#ifndef WOWMAPVIEWERREVIVED_FIRSTPERSONCAMERA_H
#define WOWMAPVIEWERREVIVED_FIRSTPERSONCAMERA_H

#include "mathfu/vector.h"
#include "mathfu/glsl_mappings.h"

class FirstPersonCamera {
public:
    FirstPersonCamera(){};


    void addDepthDiff(float val);

private:
    mathfu::vec3 camera = {0, 0, 0};
    float MDDepthPlus = 0;
    float MDDepthMinus = 0;
    float MDHorizontalPlus = 0;
    float MDHorizontalMinus = 0;

    float MDVerticalPlus = 0;
    float MDVerticalMinus = 0;

    float depthDiff = 0;

    bool staticCamera = false;

    float ah = 0;
    float av = 0;

    void addHorizontalViewDir(float val);

    void addVerticalViewDir(float val);

    void startMovingForward();

    void stopMovingForward();

    void startMovingBackwards();

    void stopMovingBackwards();

    void startStrafingLeft();

    void stopStrafingLeft();

    void startStrafingRight();

    void stopStrafingRight();

    void startMovingUp();

    void stopMovingUp();

    void startMovingDown();

    void stopMovingDown();

    void tick(float timeDelta);

    void setCameraPos(float x, float y, float z);
};


#endif //WOWMAPVIEWERREVIVED_FIRSTPERSONCAMERA_H