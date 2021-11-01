#ifndef GAMELIB_GAMESTATE_HPP
#define GAMELIB_GAMESTATE_HPP

#include "scenerendererstate.hpp"

#include <Urho3D/Graphics/Material.h>
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

    // Called from App
    void addDecalsRecursively(Urho3D::Node* node, Urho3D::Frustum const& frustum, Urho3D::Material* mat, Urho3D::Vector3 const& pos, Urho3D::Quaternion const& rot, float size, float aspect, float depth, Urho3D::Vector2 const& uv_begin, Urho3D::Vector2 const& uv_end);

private:

    unsigned controlled_node_id;

    float yaw, pitch;
    bool get_yaw_and_pitch_from_gameobject;

    unsigned decals_total;

    void handleKeyDown(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data);
    void handleUpdate(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data);
    void handleComponentAdded(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data);
    void handleSetControlledNode(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data);
    void handleCustomNetworkEvent(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data);

    unsigned reduceDecalsRecursively(Urho3D::Node* node);
};

}

#endif
