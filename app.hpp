#ifndef GAMELIB_APP_HPP
#define GAMELIB_APP_HPP

#include "../urhoextras/states/statemanager.hpp"

#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Scene/Scene.h>

namespace GameLib
{

class GameState;

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

    virtual uint16_t getDefaultPort();
    virtual Urho3D::String getDefaultHost();

    virtual Urho3D::Color getFogColor();
    virtual Urho3D::Color getAmbientLight();

    virtual Urho3D::Vector3 snapPosition(Urho3D::Vector3 const& pos);
    virtual float snapAngle(float angle);

    // This is called by GameState
    void setGameState(GameState* gamestate);

protected:

    void addDecalToGameObjects(Urho3D::Material* mat, Urho3D::Vector3 const& pos, Urho3D::Vector3 const& dir, float size, float aspect, float depth, Urho3D::Vector2 const& uv_begin, Urho3D::Vector2 const& uv_end);
    void addDecalToGameObjects(Urho3D::Material* mat, Urho3D::Vector3 const& pos, Urho3D::Quaternion const& rot, float size, float aspect, float depth, Urho3D::Vector2 const& uv_begin, Urho3D::Vector2 const& uv_end);

private:

    // Arguments from command line
    // For server
    int arg_server_port;
    // For client
    Urho3D::String arg_client_host;
    int arg_client_port;
    // For editor
    Urho3D::String arg_editor_path;

    Urho3D::SharedPtr<Urho3D::Scene> scene;

    GameState* gamestate;

    void readArguments();

    void initHeadless();
    void initWindow();
};

}

#endif
