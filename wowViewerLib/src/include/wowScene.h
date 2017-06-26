#ifndef WOWMAPVIEWERREVIVED_WOWSCENE_H_H
#define WOWMAPVIEWERREVIVED_WOWSCENE_H_H

#include <string>
#include "config.h"
#include "controllable.h"

class IFileRequest {
public:
    virtual void requestFile(const char* fileName) = 0;
};

class IFileRequester {
public:
    virtual void setFileRequestProcessor(IFileRequest * requestProcessor) = 0;
    virtual void provideFile(const char* fileName, unsigned char* data, int fileLength) = 0;
    virtual void rejectFile(const char* fileName) = 0;
};

class WoWScene : public IFileRequester {

public:
    virtual void draw(int deltaTime) = 0;

    virtual IControllable* getCurrentContollable() = 0;
};

WoWScene * createWoWScene(Config *config, IFileRequest * requestProcessor, int canvWidth, int canvHeight);

#endif //WOWMAPVIEWERREVIVED_WOWSCENE_H_H