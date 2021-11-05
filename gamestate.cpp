#include "gamestate.hpp"

#include "app.hpp"
#include "gameobject.hpp"
#include "network.hpp"

#include <Urho3D/Audio/Audio.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundListener.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/DecalSet.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/SceneEvents.h>

#include <ctime>

namespace GameLib
{

unsigned const MAX_DECALS = 10000;

GameState::GameState(App* app, Urho3D::Context* context, Urho3D::String const& host, uint16_t port) :
    SceneRendererState(app, context),
    controlled_node_id(0),
    yaw(0),
    pitch(0),
    get_yaw_and_pitch_from_gameobject(false),
    decals_total(0)
{
    // Camera and listener
    Urho3D::Node* camera_node = app->getScene()->CreateChild("camera", Urho3D::LOCAL);
    Urho3D::Camera* camera = camera_node->CreateComponent<Urho3D::Camera>();
    camera->SetFarClip(2100);
    Urho3D::SoundListener* listener = camera_node->CreateComponent<Urho3D::SoundListener>();
    GetSubsystem<Urho3D::Audio>()->SetListener(listener);

    app->initializeSceneOnClient();

    // Connect to server
    if (!GetSubsystem<Urho3D::Network>()->Connect(host, port, app->getScene())) {
        throw std::runtime_error("Unable to connect to server!");
    }
}

void GameState::show()
{
    prepareSceneForRendering();

    // Subscribe to events
    SubscribeToEvent(Urho3D::E_KEYDOWN, URHO3D_HANDLER(GameState, handleKeyDown));
    SubscribeToEvent(Urho3D::E_UPDATE, URHO3D_HANDLER(GameState, handleUpdate));
    SubscribeToEvent(Urho3D::E_COMPONENTADDED, URHO3D_HANDLER(GameState, handleComponentAdded));
    SubscribeToEvent(E_TO_CLIENT_SET_CONTROLLED_NODE, URHO3D_HANDLER(GameState, handleSetControlledNode));
    GetSubsystem<Urho3D::Network>()->RegisterRemoteEvent(E_TO_CLIENT_SET_CONTROLLED_NODE);

    // Subscribe to custom network events
    Urho3D::Vector<Urho3D::StringHash> network_events;
    getApp()->getClientNetworkEvents(network_events);
    for (unsigned i = 0; i < network_events.Size(); ++ i) {
        Urho3D::StringHash const& network_event = network_events[i];
        SubscribeToEvent(network_event, URHO3D_HANDLER(GameState, handleCustomNetworkEvent));
        GetSubsystem<Urho3D::Network>()->RegisterRemoteEvent(network_event);
    }

    // Hide mouse cursor
    GetSubsystem<Urho3D::Input>()->SetMouseVisible(false);

    getApp()->setGameState(this);
}

void GameState::hide()
{
    // Unsubscribe from events
    UnsubscribeFromEvent(Urho3D::E_KEYDOWN);
    UnsubscribeFromEvent(Urho3D::E_UPDATE);
    UnsubscribeFromEvent(Urho3D::E_COMPONENTADDED);
    UnsubscribeFromEvent(E_TO_CLIENT_SET_CONTROLLED_NODE);

    // Unsubscribe from custom network events
    Urho3D::Vector<Urho3D::StringHash> network_events;
    getApp()->getClientNetworkEvents(network_events);
    for (unsigned i = 0; i < network_events.Size(); ++ i) {
        Urho3D::StringHash const& network_event = network_events[i];
        UnsubscribeFromEvent(network_event);
    }

    getApp()->setGameState(NULL);
}

void GameState::addDecalsRecursively(Urho3D::Node* node, Urho3D::Frustum const& frustum, Urho3D::Material* mat, Urho3D::Vector3 const& pos, Urho3D::Quaternion const& rot, float size, float aspect, float depth, Urho3D::Vector2 const& uv_begin, Urho3D::Vector2 const& uv_end)
{
    // Call children recursively
    for (unsigned child_i = 0; child_i < node->GetNumChildren(); ++ child_i) {
        Urho3D::Node* child = node->GetChild(child_i);
        addDecalsRecursively(child, frustum, mat, pos, rot, size, aspect, depth, uv_begin, uv_end);
    }
    // Add all drawable components in this node
    Urho3D::DecalSet* decalset = NULL;
    Urho3D::Vector<Urho3D::SharedPtr<Urho3D::Component> > comps = node->GetComponents();
    for (unsigned comp_i = 0; comp_i < comps.Size(); ++ comp_i) {
        Urho3D::Component* comp = comps[comp_i];
        Urho3D::Drawable* drawable = dynamic_cast<Urho3D::Drawable*>(comp);
        Urho3D::DecalSet* decalset_check = dynamic_cast<Urho3D::DecalSet*>(comp);
        if (drawable && !decalset_check && frustum.IsInside(drawable->GetWorldBoundingBox()) != Urho3D::OUTSIDE) {
            if (!decalset) {
                decalset = node->GetComponent<Urho3D::DecalSet>();
                if (decalset) {
                    if (decalset->GetMaterial() != mat) {
                        URHO3D_LOGERROR("Currently only one decal set material is supported per node!");
                        return;
                    }
                } else {
                    decalset = drawable->GetNode()->CreateComponent<Urho3D::DecalSet>();
                    decalset->SetMaterial(mat);
                }
            }
            if (decalset->AddDecal(drawable, pos, rot, size, aspect, depth, uv_begin, uv_end)) {
                // Keep track of total decal count
                ++ decals_total;
            }
        }
    }
}

void GameState::handleKeyDown(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data)
{
    (void)event_type;
    int key = event_data[Urho3D::KeyDown::P_KEY].GetInt();

    if (key == Urho3D::KEY_ESCAPE) {
        getStateManager()->popState();
    }
}

void GameState::handleUpdate(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data)
{
    (void)event_type;

    float deltatime = event_data[Urho3D::Update::P_TIMESTEP].GetFloat();

    Urho3D::Connection* conn = GetSubsystem<Urho3D::Network>()->GetServerConnection();

    // Send controls to server
    if (conn) {
        Urho3D::Input* input = GetSubsystem<Urho3D::Input>();
        Urho3D::Controls controls;

        Urho3D::Node* controlled_node = getApp()->getScene()->GetNode(controlled_node_id);

        if (controlled_node) {

            // If this is the first time the player see their gameobject, then get yaw from it
            if (get_yaw_and_pitch_from_gameobject) {
                yaw = controlled_node->GetRotation().YawAngle();
                get_yaw_and_pitch_from_gameobject = false;
            }

            // Read keyboard
            controls.Set(CTRL_FORWARD, input->GetKeyDown(Urho3D::KEY_W));
            controls.Set(CTRL_BACKWARD, input->GetKeyDown(Urho3D::KEY_S));
            controls.Set(CTRL_LEFT, input->GetKeyDown(Urho3D::KEY_A));
            controls.Set(CTRL_RIGHT, input->GetKeyDown(Urho3D::KEY_D));

            // Read mouse
            Urho3D::IntVector2 mouse_move = input->GetMouseMove();
            yaw += 0.2 * mouse_move.x_;
            pitch += 0.2 * mouse_move.y_;
            controls.yaw_ = yaw;
            controls.pitch_ = pitch;
            controls.Set(CTRL_FIRE, input->GetMouseButtonDown(Urho3D::MOUSEB_LEFT));

            conn->SetControls(controls);

            // Let possible GameObject in the controlled node set the camera transform
            Urho3D::Node* camera_node = getApp()->getScene()->GetChild("camera");
            for (unsigned i = 0; i < controlled_node->GetNumComponents(); ++ i) {
                Urho3D::Component* component = controlled_node->GetComponents()[i];
                GameObject* gameobj = dynamic_cast<GameObject*>(component);
                if (gameobj) {
                    camera_node->SetTransform(gameobj->getCameraTransform());
                    break;
                }
            }
        }
    }

    // Run game objects
    Urho3D::HashSet<Urho3D::Node*> removed_nodes;
    Urho3D::PODVector<Urho3D::Node*> children = getApp()->getScene()->GetChildren(false);
    for (unsigned i = 0; i < children.Size(); ++ i) {
        Urho3D::Node* child_node = children[i];

        // If node was removed, then skip it
        if (removed_nodes.Contains(child_node)) {
            continue;
        }

        // Run possible GameObjects in this node
        for (unsigned j = 0; j < child_node->GetNumComponents(); ++ j) {
            Urho3D::Component* component = child_node->GetComponents()[j];
            GameObject* gameobj = dynamic_cast<GameObject*>(component);
            if (gameobj) {
                if (!gameobj->runClientSide(deltatime)) {
                    child_node->Remove();
                    removed_nodes.Insert(child_node);
                    break;
                }
            }
        }
    }

    // If number of decals grows too big, then clean some of them
    if (decals_total > MAX_DECALS) {
        decals_total = reduceDecalsRecursively(getApp()->getScene());
    }
}

void GameState::handleComponentAdded(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data)
{
    (void)event_type;
    Urho3D::Component* component = static_cast<Urho3D::Component*>(event_data[Urho3D::ComponentAdded::P_COMPONENT].GetPtr());
    GameObject* gameobj = dynamic_cast<GameObject*>(component);
    if (gameobj) {
        gameobj->handleAddedToClient();
    }
}

void GameState::handleSetControlledNode(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data)
{
    (void)event_type;
    controlled_node_id = event_data[P_ID].GetUInt();
    get_yaw_and_pitch_from_gameobject = true;
}

void GameState::handleCustomNetworkEvent(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data)
{
    getApp()->handleClientNetworkEvent(event_type, event_data);
}

unsigned GameState::reduceDecalsRecursively(Urho3D::Node* node)
{
    unsigned new_decal_count = 0;

    // Call children recursively
    for (unsigned child_i = 0; child_i < node->GetNumChildren(); ++ child_i) {
        Urho3D::Node* child = node->GetChild(child_i);
        new_decal_count += reduceDecalsRecursively(child);
    }

    // Reduce decals in this node
    Urho3D::Vector<Urho3D::SharedPtr<Urho3D::Component> > comps = node->GetComponents();
    for (unsigned comp_i = 0; comp_i < comps.Size(); ++ comp_i) {
        Urho3D::Component* comp = comps[comp_i];
        Urho3D::DecalSet* decalset = dynamic_cast<Urho3D::DecalSet*>(comp);
        if (decalset) {
            decalset->RemoveDecals(1);
            // If empty, then clear the whole decalset
            unsigned decalset_decals = decalset->GetNumDecals();
            if (!decalset_decals) {
                node->RemoveComponent(comp);
            } else {
                new_decal_count += decalset_decals;
            }
        }
    }

    return new_decal_count;
}

}
