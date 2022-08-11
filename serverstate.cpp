#include "serverstate.hpp"

#include "app.hpp"
#include "gameobject.hpp"
#include "network.hpp"
#include "../urhoextras/mathutils.hpp"

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/NetworkEvents.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Resource/ResourceCache.h>

#include <csignal>
#include <ctime>
#include <stdexcept>

namespace GameLib
{

void handleStopServerSignal(int sig)
{
    (void)sig;
    ServerState::stop();
}

bool ServerState::run_server = true;

ServerState::ServerState(App* app, Urho3D::Context* context, uint16_t port) :
    UrhoExtras::States::State(context),
    app(app)
{
    // Set up signal handlers for stopping the server
    #ifndef _WIN32
    ::signal(SIGINT, &handleStopServerSignal);
    ::signal(SIGQUIT, &handleStopServerSignal);
    ::signal(SIGTERM, &handleStopServerSignal);
    #endif

    // Scene
    app->getScene()->CreateComponent<Urho3D::PhysicsWorld>();

    app->initializeSceneOnServer();

    if (app->isStopping()) {
        return;
    }

    // Subscribe to events
    SubscribeToEvent(Urho3D::E_CLIENTCONNECTED, URHO3D_HANDLER(ServerState, handleClientConnected));
    SubscribeToEvent(Urho3D::E_CLIENTDISCONNECTED, URHO3D_HANDLER(ServerState, handleClientDisconnected));

    // Subscribe to custom network events
    Urho3D::Vector<Urho3D::StringHash> network_events;
    app->getServerNetworkEvents(network_events);
    for (auto network_event : network_events) {
        SubscribeToEvent(network_event, URHO3D_HANDLER(ServerState, handleCustomNetworkEvent));
        GetSubsystem<Urho3D::Network>()->RegisterRemoteEvent(network_event);
    }

    // Start listening connections
    if (!GetSubsystem<Urho3D::Network>()->StartServer(port)) {
        throw std::runtime_error("Unable to start server!");
    }
}

void ServerState::show()
{
    SubscribeToEvent(Urho3D::E_KEYDOWN, URHO3D_HANDLER(ServerState, handleKeyDown));
    SubscribeToEvent(Urho3D::E_UPDATE, URHO3D_HANDLER(ServerState, handleUpdate));
}

void ServerState::hide()
{
    UnsubscribeFromEvent(Urho3D::E_KEYDOWN);
    UnsubscribeFromEvent(Urho3D::E_UPDATE);
}

void ServerState::removed()
{
}

void ServerState::createNodeAndGameObjectForPlayer(Player* player)
{
    Urho3D::Node* player_node = app->createNodeAndGameObjectForPlayer();
    if (!player_node) {
        return;
    }

    // Set controller
    assert(node_controllers.find(player_node->GetID()) == node_controllers.end());
    assert(player->controlled_node_id == 0);
    node_controllers[player_node->GetID()] = Urho3D::SharedPtr<Player>(player);
    player->controlled_node_id = player_node->GetID();

    player_node->SetOwner(player->conn);

    // Inform about the controlled node
    Urho3D::VariantMap event_args;
    event_args[P_ID] = player_node->GetID();
    player->conn->SendRemoteEvent(E_TO_CLIENT_SET_CONTROLLED_NODE, true, event_args);
}

void ServerState::handleKeyDown(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data)
{
    (void)event_type;
    int key = event_data[Urho3D::KeyDown::P_KEY].GetInt();

    if (key == Urho3D::KEY_ESCAPE) {
        getStateManager()->popState();
    }
}

void ServerState::stop()
{
    run_server = false;
}

void ServerState::handleUpdate(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data)
{
    (void)event_type;

    float deltatime = event_data[Urho3D::Update::P_TIMESTEP].GetFloat();
(void)deltatime;

    // If stop was requested
    if (!run_server) {
        URHO3D_LOGINFO("Shut down was requested...");
        GetSubsystem<Urho3D::Network>()->StopServer();
        getStateManager()->popState();
        return;
    }

    // Run game objects
    Urho3D::HashSet<Urho3D::Node*> removed_nodes;
    Urho3D::PODVector<Urho3D::Node*> children = app->getScene()->GetChildren(false);
    for (unsigned i = 0; i < children.Size(); ++ i) {
        Urho3D::Node* child_node = children[i];

        // If node was removed, then skip it
        if (removed_nodes.Contains(child_node)) {
            continue;
        }

        // Check if this node is controlled by somebody
        NodeControllers::iterator node_controllers_find = node_controllers.find(child_node->GetID());
        Player* player = NULL;
        if (node_controllers_find != node_controllers.end()) {
            player = node_controllers_find->second;
        }

        // Run possible GameObjects in this node
        bool node_was_destroyed = false;
        for (unsigned j = 0; j < child_node->GetNumComponents(); ++ j) {
            Urho3D::Component* component = child_node->GetComponents()[j];
            GameObject* gameobj = dynamic_cast<GameObject*>(component);
            if (gameobj) {
                Urho3D::Controls const* controls = nullptr;
                if (player && player->conn) {
                    controls = &player->conn->GetControls();
                }
                if (!gameobj->runServerSide(deltatime, controls)) {
                    child_node->Remove();
                    removed_nodes.Insert(child_node);
                    node_was_destroyed = true;
                    break;
                }
            }
        }

        // If node was destroyed and it was controlled by somebody, then initiate a respawn
        if (player && node_was_destroyed) {
            // If node belongs to a human player, then inform they no longer control it
            if (player->conn) {
                Urho3D::VariantMap event_args;
                event_args[P_ID] = 0;
                player->conn->SendRemoteEvent(E_TO_CLIENT_SET_CONTROLLED_NODE, true, event_args);
            }
            // Remove controlling
            node_controllers.erase(node_controllers_find);
            player->controlled_node_id = 0;
            // Initiate a respawn
            player->respawn_at = ::time(NULL) + 4;
        }
    }

    // Run respawns
    for (Players::iterator i = players.begin(); i != players.end(); ++ i) {
        Player* player = *i;
        // If it is time for respawn
        if (player->respawn_at > 0 && player->respawn_at < ::time(NULL)) {
            createNodeAndGameObjectForPlayer(player);
            player->respawn_at = 0;
        }
    }
}

void ServerState::handleClientConnected(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data)
{
    (void)event_type;

    Urho3D::Connection* conn = static_cast<Urho3D::Connection*>(event_data[Urho3D::ClientConnected::P_CONNECTION].GetPtr());

    Urho3D::SharedPtr<Player> player(new Player(conn));
    players.insert(player);

    conn->SetScene(app->getScene());

    createNodeAndGameObjectForPlayer(player);
}

void ServerState::handleClientDisconnected(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data)
{
    (void)event_type;

    Urho3D::Connection* conn = static_cast<Urho3D::Connection*>(event_data[Urho3D::ClientConnected::P_CONNECTION].GetPtr());

    Player* player = getPlayer(conn);
    assert(player);

    // Clean node controller
    if (player->controlled_node_id) {
        node_controllers.erase(player->controlled_node_id);
    }
    // Clean player
    players.erase(Urho3D::SharedPtr<Player>(player));
}

void ServerState::handleCustomNetworkEvent(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data)
{
    // Get connection from event data
    Urho3D::Connection* conn = static_cast<Urho3D::Connection*>(event_data[Urho3D::NetworkMessage::P_CONNECTION].GetPtr());

    app->handleServerNetworkEvent(conn, event_type, event_data);
}

Player* ServerState::getPlayer(Urho3D::Connection* conn)
{
    for (Players::iterator i = players.begin(); i != players.end(); ++ i) {
        Player* player = *i;
        if (player->conn == conn) {
            return player;
        }
    }
    return NULL;
}

}
