#ifndef GAMELIB_SCENERENDERERSTATE_H
#define GAMELIB_SCENERENDERERSTATE_H

#include "../urhoextras/states/state.hpp"

namespace GameLib
{

class App;

class SceneRendererState : public UrhoExtras::States::State
{

public:

    SceneRendererState(App* app, Urho3D::Context* context);

protected:

    App* getApp();

    void prepareSceneForRendering();

private:

    App* app;
};

}

#endif
