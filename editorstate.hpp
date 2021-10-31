#ifndef GAMELIB_EDITORSTATE_HPP
#define GAMELIB_EDITORSTATE_HPP

#include "../gamelib/gameobject.hpp"
#include "../urhoextras/cameracontrol.hpp"
#include "scenerendererstate.hpp"

#include <Urho3D/Scene/Scene.h>

namespace GameLib
{

class App;

class EditorState : public SceneRendererState
{

public:

    EditorState(App* app, Urho3D::Context* context, Urho3D::String const& path);

    void show() override;
    void hide() override;

private:

    enum Mode
    {
        MODE_DEFAULT,
        MODE_ROTATING_VIEW,
        MODE_ROTATING_OBJECT
    };

    Urho3D::String path;

    Urho3D::Vector<Urho3D::StringHash> editable_object_types;

    Mode mode;

    // Camera
    UrhoExtras::CameraControl cam_control;

    // Brush
    int brush_selection;
    float brush_yaw;

    void handleKeyDown(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data);
    void handleKeyUp(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data);
    void handleMouseButtonDown(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data);
    void handleMouseButtonUp(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data);
    void handleMouseWheel(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data);
    void handleUpdate(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data);

    bool raycast(Urho3D::Vector3& result_pos, Urho3D::Vector3& result_normal);

    Urho3D::Vector3 calculateObjectPlacementPosition(Urho3D::Vector3 const& pos, Urho3D::Vector3 const& normal, GameLib::GameObject const* obj);
};

}

#endif
