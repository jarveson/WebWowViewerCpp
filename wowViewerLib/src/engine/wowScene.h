#ifndef WOWMAPVIEWERREVIVED_WOWSCENEIMPL_H_H
#define WOWMAPVIEWERREVIVED_WOWSCENEIMPL_H_H

#include <string>
#include <functional>
#include <algorithm>
#include <cctype>
#include <locale>
#include <mutex>
#include <future>
#include "wowInnerApi.h"

#include "../include/wowScene.h"
#include "../include/config.h"
#include "mathfu/glsl_mappings.h"
#include "camera/firstPersonOrthoCamera.h"
#include "camera/firstPersonCamera.h"
#include "texture/BlpTexture.h"
#include "geometry/skinGeom.h"
#include "geometry/m2Geom.h"
#include "cache/cache.h"
#include "stringTrim.h"
#include "geometry/wmoGroupGeom.h"
#include "geometry/wmoMainGeom.h"
#include "objects/scenes/map.h"
#include "persistance/wdtFile.h"
#include "persistance/wdlFile.h"
#include "persistance/db2/base/DB2Base.h"
#include "persistance/db2/DB2Light.h"
#include "../gapi/interface/IDevice.h"
#include "objects/wowFrameData.h"
#include "camera/planarCamera.h"
#include "persistance/animFile.h"
#include "persistance/skelFile.h"

class WoWSceneImpl: public WoWScene, public IWoWInnerApi {

public:
    WoWSceneImpl(Config *config, IFileRequest * requestProcessor, IDevice * device, int canvWidth, int canvHeight);
    ~WoWSceneImpl() override;

    void draw(animTime_t deltaTime) override;
    void setScreenSize(int canvWidth, int canvHeight) override;

    void provideFile(CacheHolderType holderType, const char* fileName, const HFileContent &data) override;
    virtual void rejectFile(CacheHolderType holderType, const char* fileName) override {
        std::string s_fileName(fileName);

        adtObjectCache.reject(s_fileName);
        wmoGeomCache.reject(s_fileName);
        wmoMainCache.reject(s_fileName);
        m2GeomCache.reject(s_fileName);
        skinGeomCache.reject(s_fileName);
        textureCache.reject(s_fileName);
        wdtCache.reject(s_fileName);
        wdlCache.reject(s_fileName);
        db2Cache.reject(s_fileName);
    }
    void setFileRequestProcessor(IFileRequest*) override {} ;

    IControllable* controllable = &m_firstCamera;

    IControllable* getCurrentCamera() override {
        return this->controllable;
    };

    void switchCameras() override {
        if (controllable == &m_firstCamera) {
            this->m_config->setUseSecondCamera(true);
            controllable = &m_secondCamera;
            mathfu::vec3 camPos = m_firstCamera.getCameraPosition();
            m_secondCamera.setCameraPos(camPos.x, camPos.y, camPos.z);
        } else {
            this->m_config->setUseSecondCamera(false);
            controllable = &m_firstCamera;
        }
    }
public:
    virtual IDevice * getDevice() override {
        return m_gdevice.get();
    }

    virtual HGUniformBuffer getSceneWideUniformBuffer() override {
        return m_sceneWideUniformBuffer;
    }

    virtual Cache<AdtFile> *getAdtGeomCache() override {
        return &adtObjectCache;
    }
    virtual Cache<M2Geom> *getM2GeomCache() override {
        return &m2GeomCache;
    };
    virtual Cache<SkinGeom>* getSkinGeomCache() override {
        return &skinGeomCache;
    };
    virtual Cache<AnimFile>* getAnimCache() override {
        return &animCache;
    };
    virtual Cache<SkelFile>* getSkelCache() override {
        return &skelCache;
    };
    virtual Cache<BlpTexture> *getTextureCache() override {
        return &textureCache;
    };
    virtual Cache<WmoMainGeom>* getWmoMainCache() override {
        return &wmoMainCache;
    };
    virtual Cache<WmoGroupGeom>* getWmoGroupGeomCache() override {
        return &wmoGeomCache;
    };
    virtual Cache<WdtFile>* getWdtFileCache() override {
        return &wdtCache;
    };

    virtual Cache<WdlFile>* getWdlFileCache() override {
        return &wdlCache;
    };

    virtual mathfu::mat4& getViewMat() override {
        return m_FrameParams[updateFrameIndex].m_lookAtMat4;
    }
    virtual mathfu::vec4 getGlobalAmbientColor() override {
        return m_FrameParams[updateFrameIndex].m_globalAmbientColor ;
    }
    virtual mathfu::vec3 getGlobalSunDir() override {
        return m_FrameParams[updateFrameIndex].m_sunDir;
    };
    virtual mathfu::vec4 getGlobalSunColor() override {
        return m_FrameParams[updateFrameIndex].m_globalSunColor;
    }
    virtual mathfu::vec4 getGlobalFogColor() override {
        return m_FrameParams[updateFrameIndex].m_fogColor;
    }
    virtual float getGlobalFogStart() override { return m_FrameParams[updateFrameIndex].uFogStart; };
    virtual float getGlobalFogEnd() override { return m_FrameParams[updateFrameIndex].uFogEnd; };
    virtual mathfu::vec3 getViewUp() override { return m_FrameParams[updateFrameIndex].m_upVector; };

    int updateFrameIndex = 0;

    virtual Config *getConfig() override {
        return m_config;
    }
    virtual DB2Light *getDB2Light() override {
        return db2Light;
    };
    virtual DB2LightData *getDB2LightData() override {
        return db2LightData;
    };
    virtual DB2WmoAreaTable *getDB2WmoAreaTable() override {
        return db2WmoAreaTable;
    };
    void setScenePos(float x, float y, float z) override {
        m_firstCamera.setCameraPos(x,y,z);
    };
    void setCameraPosition(float x, float y, float z) override {
        m_planarCamera.setCameraPos(x,y,z);
    }
    void setCameraOffset(float x, float y, float z) override {
        m_planarCamera.setCameraOffset(x,y,z);
    };
    void clearCache() override;

    void setAnimationId(int animationId) override;
    void setReplaceTextureArray(std::vector<int> &replaceTextureArray) override;

    void setScene(int sceneType, std::string fileName, int cameraNum) override;
    void setSceneWithFileDataId(int sceneType, int fileDataId, int cameraNum) override;
    void setMap(int mapId, int wdtFileId, float x, float y, float z) override;
private:
    void DoCulling();
    void actuallDropCache();
private:
    bool m_enable;

    bool m_isTerminating = false;
    Config * m_config;
    std::unique_ptr<IDevice> m_gdevice;
    HGUniformBuffer m_sceneWideUniformBuffer;

    WoWFrameData m_FrameParams[4];

    bool m_usePlanarCamera = false;

    FirstPersonCamera m_firstCamera;
    PlanarCamera m_planarCamera;
    FirstPersonCamera m_secondCamera;

    DB2Light *db2Light;
    DB2LightData *db2LightData;
    DB2WmoAreaTable *db2WmoAreaTable;

    int canvWidth;
    int canvHeight;
    float canvAspect;


    bool m_isDebugCamera = false;

    Cache<AdtFile> adtObjectCache;
    Cache<WdtFile> wdtCache;
    Cache<WdlFile> wdlCache;
    Cache<WmoGroupGeom> wmoGeomCache;
    Cache<WmoMainGeom> wmoMainCache;
    Cache<M2Geom> m2GeomCache;
    Cache<SkinGeom> skinGeomCache;
    Cache<BlpTexture> textureCache;
    Cache<DB2Base> db2Cache;
    Cache<AnimFile> animCache;
    Cache<SkelFile> skelCache;

    iInnerSceneApi *currentScene = nullptr;
    iInnerSceneApi *newScene = nullptr;

    bool needToDropCache = false;
    std::promise<float> nextDeltaTime = std::promise<float>();
    std::promise<float> nextDeltaTimeForUpdate;
    std::promise<bool> cullingFinished;
    std::promise<bool> updateFinished;

//    void drawTexturedQuad(GLuint texture, float x, float y, float width, float height, float canv_width, float canv_height,
//                          bool drawDepth);

    void drawCamera() override ;
    bool getIsDebugCamera() override {
        return m_isDebugCamera;
    }

    bool m_supportThreads = true;

    void SetDirection(WoWFrameData &frameParamHolder);

    void processCaches(int limit);
};



#endif //WOWMAPVIEWERREVIVED_WOWSCENEIMPL_H_H
