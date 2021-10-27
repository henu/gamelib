#ifndef GAMELIB_APP_HPP
#define GAMELIB_APP_HPP

#include "../urhoextras/states/statemanager.hpp"

#include <Urho3D/Scene/Scene.h>

namespace GameLib
{

class App : public UrhoExtras::States::StateManager
{

public:

    App(Urho3D::Context* context);

    void Start() override;

    Urho3D::Scene* getScene();

    virtual void initializeSceneOnServer();
    virtual void initializeSceneOnClient();

    virtual Urho3D::Node* createNodeAndGameObjectForPlayer();

    virtual void getClientNetworkEvents(Urho3D::Vector<Urho3D::StringHash>& result);
    virtual void handleClientNetworkEvent(Urho3D::StringHash const& event_type, Urho3D::VariantMap& event_data);

private:

    // Arguments from command line
    // For server
    int arg_server_port;
    // For client
    Urho3D::String arg_client_host;
    int arg_client_port;

    Urho3D::SharedPtr<Urho3D::Scene> scene;

    void readArguments();

    void initHeadless();
    void initWindow();
};

}

#endif
