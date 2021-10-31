#ifndef GAMELIB_SCENESERIALIZER_HPP
#define GAMELIB_SCENESERIALIZER_HPP

#include <Urho3D/Scene/Scene.h>

namespace GameLib
{

void readSceneFromDisk(Urho3D::Scene* scene, Urho3D::String const& path, bool enable_physics);
void writeSceneToDisk(Urho3D::Scene* scene, Urho3D::String const& path);

}

#endif
