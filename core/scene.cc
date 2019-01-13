//
// Created by xiaoc on 2019/1/12.
//

#include "scene.h"
using namespace Miyuki;
Scene::Scene() {
    rtcScene = rtcNewScene(GetEmbreeDevice());
}

Scene::~Scene() {
    rtcReleaseScene(rtcScene);
}

void Scene::commit() {
    rtcCommitScene(rtcScene);
}
