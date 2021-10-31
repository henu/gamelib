#ifndef GAMELIB_GAMESTATE_HPP
#define GAMELIB_GAMESTATE_HPP

#include "scenerendererstate.hpp"

#include <Urho3D/Scene/Scene.h>

namespace GameLib
{

class App;

class GameState : public SceneRendererState
{

public:

    GameState(App* app, Urho3D::Context* context, Urho3D::String const& host, uint16_t port);

    void show() override;
    void hide() override;

private:

    unsigned controlled_node_id;

    float yaw, pitch;
    bool get_yaw_and_pitch_from_gameobject;

    void handleKeyDown(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data);
    void handleUpdate(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data);
    void handleComponentAdded(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data);
    void handleSetControlledNode(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data);
    void handleCustomNetworkEvent(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data);
};

}

#endif
