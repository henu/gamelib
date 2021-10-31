#include "editorstate.hpp"

#include "../urhoextras/mathutils.hpp"
#include "app.hpp"
#include "sceneserializer.hpp"

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Scene/Scene.h>

namespace GameLib
{

float const MOVEMENT_SPEED = 10;

EditorState::EditorState(App* app, Urho3D::Context* context, Urho3D::String const& path) :
    SceneRendererState(app, context),
    path(path),
    cam_control(context),
    cam_rotating(false),
    brush_selection(-1)
{
    // Camera and listener
    cam_control.setPitch(45);
    Urho3D::Node* camera_node = app->getScene()->CreateChild("camera", Urho3D::LOCAL);
    camera_node->SetPosition(Urho3D::Vector3(0, 10, 0));
    camera_node->SetRotation(cam_control.getRotation());
    Urho3D::Camera* camera = camera_node->CreateComponent<Urho3D::Camera>();
    camera->SetFarClip(2100);

    // Sun
    Urho3D::Node* sun_node = app->getScene()->CreateChild();
    sun_node->Rotate(Urho3D::Quaternion(60, Urho3D::Vector3::RIGHT));
    sun_node->Rotate(Urho3D::Quaternion(30, Urho3D::Vector3::UP));
    Urho3D::Light* sun = sun_node->CreateComponent<Urho3D::Light>();
    sun->SetLightType(Urho3D::LIGHT_DIRECTIONAL);
    sun->SetCastShadows(true);

    // Get all editable gameobjects
    Urho3D::HashMap<Urho3D::String, Urho3D::Vector<Urho3D::StringHash> > categories = context->GetObjectCategories();
    if (categories.Contains("editable")) {
        editable_object_types = categories["editable"];
    }

    // Read scene, if it exists
    Urho3D::FileSystem* fs = GetSubsystem<Urho3D::FileSystem>();
    if (fs->FileExists(path)) {
        readSceneFromDisk(getApp()->getScene(), path, false);
    }
}

void EditorState::show()
{
    prepareSceneForRendering();

    // Subscribe to events
    SubscribeToEvent(Urho3D::E_KEYDOWN, URHO3D_HANDLER(EditorState, handleKeyDown));
    SubscribeToEvent(Urho3D::E_MOUSEBUTTONDOWN, URHO3D_HANDLER(EditorState, handleMouseButtonDown));
    SubscribeToEvent(Urho3D::E_MOUSEBUTTONUP, URHO3D_HANDLER(EditorState, handleMouseButtonUp));
    SubscribeToEvent(Urho3D::E_MOUSEWHEEL, URHO3D_HANDLER(EditorState, handleMouseWheel));
    SubscribeToEvent(Urho3D::E_UPDATE, URHO3D_HANDLER(EditorState, handleUpdate));

    GetSubsystem<Urho3D::Input>()->SetMouseVisible(true);
}

void EditorState::hide()
{
    // Remove possible brush gameobject
    Urho3D::Node* brush_node = getApp()->getScene()->GetChild("brush");
    if (brush_node) {
        brush_node->Remove();
    }

    // Write scene to disk
    writeSceneToDisk(getApp()->getScene(), path);

    // Unsubscribe from events
    UnsubscribeFromEvent(Urho3D::E_KEYDOWN);
    UnsubscribeFromEvent(Urho3D::E_MOUSEBUTTONDOWN);
    UnsubscribeFromEvent(Urho3D::E_MOUSEBUTTONUP);
    UnsubscribeFromEvent(Urho3D::E_MOUSEWHEEL);
    UnsubscribeFromEvent(Urho3D::E_UPDATE);
}


void EditorState::handleKeyDown(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data)
{
    (void)event_type;
    (void)event_data;
}

void EditorState::handleMouseButtonDown(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data)
{
    (void)event_type;

    int button = event_data[Urho3D::MouseButtonDown::P_BUTTON].GetInt();

    if (button == Urho3D::MOUSEB_LEFT) {
        if (!cam_rotating) {
            if (brush_selection >= 0) {
                Urho3D::Node* brush_node = getApp()->getScene()->GetChild("brush");
                Urho3D::Node* node = getApp()->getScene()->CreateChild();
                node->SetTransform(brush_node->GetTransform());
                Urho3D::Component* obj_raw = node->CreateComponent(editable_object_types[brush_selection]);
                GameObject* obj = dynamic_cast<GameObject*>(obj_raw);
                obj->finishCreation(false);
            }
        }
    } else if (button == Urho3D::MOUSEB_RIGHT) {
        cam_rotating = true;
        GetSubsystem<Urho3D::Input>()->SetMouseVisible(false);
    }
}

void EditorState::handleMouseButtonUp(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data)
{
    (void)event_type;

    int button = event_data[Urho3D::MouseButtonDown::P_BUTTON].GetInt();

    if (button == Urho3D::MOUSEB_RIGHT) {
        cam_rotating = false;
        GetSubsystem<Urho3D::Input>()->SetMouseVisible(true);
    }
}

void EditorState::handleMouseWheel(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data)
{
    (void)event_type;

    int wheel = event_data[Urho3D::MouseWheel::P_WHEEL].GetInt();
    brush_selection += wheel;
    while (brush_selection < -1) {
        brush_selection += editable_object_types.Size() + 1;
    }
    while (brush_selection >= int(editable_object_types.Size())) {
        brush_selection -= editable_object_types.Size() + 1;
    }
    Urho3D::Node* brush_node = getApp()->getScene()->GetChild("brush");
    if (brush_node) {
        brush_node->Remove();
    }
}

void EditorState::handleUpdate(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data)
{
    (void)event_type;

    float deltatime = event_data[Urho3D::Update::P_TIMESTEP].GetFloat();

    // Update camera transform
    cam_control.update(cam_rotating);
    Urho3D::Node* cam_node = getApp()->getScene()->GetChild("camera");
    cam_node->SetRotation(cam_control.getRotation());
    cam_node->SetPosition(cam_node->GetPosition() + cam_control.getFlyingMovement() * deltatime * MOVEMENT_SPEED);

    // Update brush "cursor"
    Urho3D::Vector3 brush_pos;
    Urho3D::Vector3 brush_normal;
    Urho3D::Node* brush_node = getApp()->getScene()->GetChild("brush");
    bool brush_visible = raycast(brush_pos, brush_normal);
    if (brush_visible && brush_selection >= 0 && !cam_rotating) {
        GameObject* obj = NULL;
        if (!brush_node) {
            brush_node = getApp()->getScene()->CreateChild("brush");
            Urho3D::Component* obj_raw = brush_node->CreateComponent(editable_object_types[brush_selection]);
            obj = dynamic_cast<GameObject*>(obj_raw);
            obj->finishCreation(false);
        } else {
            for (unsigned i = 0; i < brush_node->GetNumComponents(); ++ i) {
                Urho3D::Component* component = brush_node->GetComponents()[i];
                obj = dynamic_cast<GameObject*>(component);
                if (obj) {
                    break;
                }
            }
            assert(obj);
        }
        if (obj) {
            brush_node->SetPosition(calculateObjectPlacementPosition(brush_pos, brush_normal, obj));
        }
    } else {
        if (brush_node) {
            brush_node->Remove();
        }
    }

}

bool EditorState::raycast(Urho3D::Vector3& result_pos, Urho3D::Vector3& result_normal)
{
    Urho3D::Graphics* graphics = GetSubsystem<Urho3D::Graphics>();
    Urho3D::Input* input = GetSubsystem<Urho3D::Input>();

    Urho3D::Node* camera_node = getApp()->getScene()->GetChild("camera");
    Urho3D::Camera* camera = camera_node->GetComponent<Urho3D::Camera>();
    Urho3D::IntVector2 mouse_pos = input->GetMousePosition();
    Urho3D::Ray mouse_ray = camera->GetScreenRay((float)mouse_pos.x_ / graphics->GetWidth(), (float)mouse_pos.y_ / graphics->GetHeight());
    Urho3D::PODVector<Urho3D::RayQueryResult> raycast_results;
    Urho3D::RayOctreeQuery raycast_query(raycast_results, mouse_ray, Urho3D::RAY_TRIANGLE, Urho3D::M_INFINITY, Urho3D::DRAWABLE_GEOMETRY);
    getApp()->getScene()->GetComponent<Urho3D::Octree>()->Raycast(raycast_query);

    // If there was a hit to some object
    for (unsigned i = 0; i < raycast_results.Size(); ++ i) {
        Urho3D::RayQueryResult raycast_result = raycast_results[i];
        // Skip brush object
        if (raycast_result.drawable_->GetNode() == getApp()->getScene()->GetChild("brush")) {
            continue;
        }
        result_pos = raycast_result.position_;
        result_normal = raycast_result.normal_;
        return true;
    }
    // If there was no hit to any object, then check if ground was hit
    if (mouse_ray.origin_.y_ > 0 && mouse_ray.direction_.y_ < 0) {
        Urho3D::Plane ground(Urho3D::Vector3::UP, Urho3D::Vector3::ZERO);
        result_pos = UrhoExtras::projectToPlaneWithDirection(mouse_ray.origin_, ground, mouse_ray.direction_);
        // If result is too far, then do not accept the hit
        if (result_pos.x_ > 9999 || result_pos.x_ < -9999 || result_pos.z_ > 9999 || result_pos.z_ < -9999) {
            return false;
        }
        result_normal = Urho3D::Vector3::UP;
        return true;
    }
    return false;
}

Urho3D::Vector3 EditorState::calculateObjectPlacementPosition(Urho3D::Vector3 const& pos, Urho3D::Vector3 const& normal, GameObject const* obj)
{
    Shape shape = obj->getPlacementShape();

    return pos + shape.positionAtNormal(normal);
}

}
