#include "sceneserializer.hpp"

#include "app.hpp"
#include "gameobject.hpp"

#include <Urho3D/IO/File.h>
#include <Urho3D/IO/Log.h>
#include <stdexcept>

namespace GameLib
{

uint16_t const VERSION_0_INITIAL = 0;

void readSceneFromDisk(App* app, Urho3D::String const& path, bool enable_physics)
{
    Urho3D::Context* context = app->GetContext();

    Urho3D::File file(context, path, Urho3D::FILE_READ);

    Urho3D::HashMap<Urho3D::String, Urho3D::Vector<Urho3D::StringHash> > categories = context->GetObjectCategories();
    if (!categories.Contains("editable")) {
        throw std::runtime_error("No game objects are defined as editable!");
    }
    Urho3D::Vector<Urho3D::StringHash> editable_object_types = categories["editable"];

    // Check header and version
    char header_check[12];
    file.Read(header_check, 12);
    if (::strncmp(header_check, "GameLibScene", 12)) {
        throw std::runtime_error("Not a scene file!");
    }
    uint16_t version_check = file.ReadUShort();
    if (version_check != VERSION_0_INITIAL) {
        throw std::runtime_error("Unsupported version!");
    }

    // Read gameobjects
    unsigned gameobjs_count = file.ReadUInt();
    for (unsigned i = 0; i < gameobjs_count; ++ i) {
        Urho3D::StringHash type = file.ReadStringHash();
        Urho3D::Matrix3x4 transf = file.ReadMatrix3x4();
        // Only allow predefined objects
        if (editable_object_types.Contains(type)) {
            Urho3D::Node* node = app->getScene()->CreateChild();
            node->SetTransform(transf);
            Urho3D::Component* obj_raw = node->CreateComponent(type);
            GameObject* obj = dynamic_cast<GameObject*>(obj_raw);
            if (obj) {
                obj->finishCreation(app, enable_physics);
            } else {
                URHO3D_LOGERROR("Scene contains components that are not gameobjects!");
            }
        } else {
            URHO3D_LOGERROR("Scene contains gameobject that is not defined as editable!");
        }
    }
}

void writeSceneToDisk(Urho3D::Scene* scene, Urho3D::String const& path)
{
    Urho3D::File file(scene->GetContext(), path, Urho3D::FILE_WRITE);

    // Header
    file.Write("GameLibScene", 12);
    file.WriteUShort(VERSION_0_INITIAL);

    // Count number of game objects
    unsigned gameobjs_count = 0;
    Urho3D::PODVector<Urho3D::Node*> children = scene->GetChildren(false);
    for (unsigned i = 0; i < children.Size(); ++ i) {
        Urho3D::Node* node = children[i];
        for (unsigned j = 0; j < node->GetNumComponents(); ++ j) {
            Urho3D::Component* component = node->GetComponents()[j];
            GameObject* gameobj = dynamic_cast<GameObject*>(component);
            if (gameobj) {
                ++ gameobjs_count;
            }
        }
    }

    // Write game objects
    file.WriteUInt(gameobjs_count);
    for (unsigned i = 0; i < children.Size(); ++ i) {
        Urho3D::Node* node = children[i];
        for (unsigned j = 0; j < node->GetNumComponents(); ++ j) {
            Urho3D::Component* component = node->GetComponents()[j];
            GameObject* gameobj = dynamic_cast<GameObject*>(component);
            if (gameobj) {
                // Type
                file.WriteStringHash(gameobj->GetType());
                // Transform
                file.WriteMatrix3x4(node->GetTransform());
            }
        }
    }
}

}
