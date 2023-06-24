#ifndef GAMELIB_SERVERSTATE_HPP
#define GAMELIB_SERVERSTATE_HPP

#include "player.hpp"
#include "../urhoextras/states/state.hpp"

#include <Urho3D/Network/Connection.h>
#include <Urho3D/Scene/Scene.h>

#include <map>
#include <set>
#include <cstdint>

namespace GameLib
{

class App;

class ServerState : public UrhoExtras::States::State
{

public:

    ServerState(App* app, Urho3D::Context* context, uint16_t port);

    void show() override;
    void hide() override;
    void removed() override;

    static void stop();

private:

    typedef std::set<Urho3D::SharedPtr<Player> > Players;

    // Mapping from Node to its controller
    typedef std::map<unsigned, Urho3D::SharedPtr<Player> > NodeControllers;

    App* app;

    static bool run_server;

    Players players;
    NodeControllers node_controllers;

    void createNodeAndGameObjectForPlayer(Player* player);

    void handleKeyDown(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data);
    void handleUpdate(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data);
    void handleClientConnected(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data);
    void handleClientDisconnected(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data);
    void handlePhysicsCollision(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data);
    void handleSetPlayerName(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data);

    void handleCustomNetworkEvent(Urho3D::StringHash event_type, Urho3D::VariantMap& event_data);

    Player* getPlayer(Urho3D::Connection* conn);
};

}

#endif
