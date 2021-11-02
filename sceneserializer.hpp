#ifndef GAMELIB_SCENESERIALIZER_HPP
#define GAMELIB_SCENESERIALIZER_HPP

#include <Urho3D/Scene/Scene.h>

namespace GameLib
{

class App;

void readSceneFromDisk(App* app, Urho3D::String const& path, bool enable_physics = true);
void writeSceneToDisk(Urho3D::Scene* scene, Urho3D::String const& path);

}

#endif
