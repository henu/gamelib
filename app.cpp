#include "app.hpp"

#include "gamestate.hpp"
#include "serverstate.hpp"
#include "editorstate.hpp"

#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/IO/Log.h>

#include <stdexcept>

namespace GameLib
{

App::App(Urho3D::Context* context) :
    UrhoExtras::States::StateManager(context),
    arg_server_port(0),
    arg_client_port(0)
{
    try {
        readArguments();
    } catch (std::runtime_error const& err) {
        initHeadless();
        URHO3D_LOGERROR(err.what());
        engine_->Exit();
        return;
    }

    // If server or map conversion
    if (arg_server_port > 0) {
        initHeadless();
    }
    // If client or map editor
    else {
        initWindow();
    }
}

void App::Start()
{
    if (engine_->IsExiting()) {
        return;
    }

    Urho3D::SetRandomSeed(Urho3D::Time::GetSystemTime());

    scene = new Urho3D::Scene(context_);
    scene->CreateComponent<Urho3D::Octree>(Urho3D::LOCAL);

    // If server
    if (arg_server_port > 0) {
        pushState(Urho3D::SharedPtr<ServerState>(new ServerState(this, context_, arg_server_port)));
    }
    // If client
    else if (arg_client_port > 0) {
        pushState(Urho3D::SharedPtr<GameState>(new GameState(this, context_, arg_client_host, arg_client_port)));
    }
    // If scene editor
    else if (!arg_editor_path.Empty()) {
        pushState(Urho3D::SharedPtr<EditorState>(new EditorState(this, context_, arg_editor_path)));
    }
    // If no arguments are given, then connect to default server
    else {
        pushState(Urho3D::SharedPtr<GameState>(new GameState(this, context_, getDefaultHost(), getDefaultPort())));
    }
}

Urho3D::Scene* App::getScene()
{
    return scene;
}

void App::initializeSceneOnServer()
{
}

void App::initializeSceneOnClient()
{
}

Urho3D::Node* App::createNodeAndGameObjectForPlayer()
{
    return NULL;
}

void App::getClientNetworkEvents(Urho3D::Vector<Urho3D::StringHash>& result)
{
    (void)result;
}

void App::handleClientNetworkEvent(Urho3D::StringHash const& event_type, Urho3D::VariantMap& event_data)
{
    (void)event_type;
    (void)event_data;
}

uint16_t App::getDefaultPort()
{
    return 2345;
}

Urho3D::String App::getDefaultHost()
{
    return "localhost";
}

Urho3D::Color App::getFogColor()
{
    return Urho3D::Color::BLACK;
}

Urho3D::Color App::getAmbientLight()
{
    return Urho3D::Color(0.35, 0.35, 0.35);
}

Urho3D::Vector3 App::snapPosition(Urho3D::Vector3 const& pos)
{
    return Urho3D::Vector3(Urho3D::Round(pos.x_), Urho3D::Round(pos.y_), Urho3D::Round(pos.z_));
}

float App::snapAngle(float angle)
{
    return Urho3D::Round(angle / 22.5) * 22.5;
}

void App::readArguments()
{
    // Do the reading
    try {
        Urho3D::Vector<Urho3D::String> const& args = Urho3D::GetArguments();
        for (unsigned i = 0; i < args.Size(); ++ i) {
            Urho3D::String const& arg = args[i];
            // Server
            if (arg == "listen") {
                if (arg_server_port > 0) {
                    throw std::runtime_error("Duplicate \"listen\"!");
                }
                if (arg_client_port > 0 || !arg_editor_path.Empty()) {
                    throw std::runtime_error("Please select either \"listen\", \"connect\" or \"editor\"!");
                }
                if (args.Size() - i < 2) {
                    throw std::runtime_error("Missing port!");
                }
                arg_server_port = Urho3D::ToInt(args[i + 1]);
                if (arg_server_port < 1 || arg_server_port > 65535) {
                    throw std::runtime_error("Port must be between 1 and 65535!");
                }
                ++ i;
            }
            // Client
            else if (arg == "connect") {
                if (arg_client_port > 0) {
                    throw std::runtime_error("Duplicate \"connect\"!");
                }
                if (arg_server_port > 0 || !arg_editor_path.Empty()) {
                    throw std::runtime_error("Please select either \"listen\", \"connect\" or \"editor\"!");
                }
                if (args.Size() - i < 2) {
                    throw std::runtime_error("Missing hostname and port!");
                }
                if (args.Size() - i < 3) {
                    throw std::runtime_error("Missing port!");
                }
                arg_client_host = args[i + 1];
                arg_client_port = Urho3D::ToInt(args[i + 2]);
                if (arg_client_port < 1 || arg_client_port > 65535) {
                    throw std::runtime_error("Port must be between 1 and 65535!");
                }
                i += 2;
            }
            // Scene editor
            else if (arg == "editor") {
                if (!arg_editor_path.Empty()) {
                    throw std::runtime_error("Duplicate \"editor\"!");
                }
                if (arg_server_port > 0 || arg_client_port > 0) {
                    throw std::runtime_error("Please select either \"listen\", \"connect\" or \"editor\"!");
                }
                if (args.Size() - i < 2) {
                    throw std::runtime_error("Missing scene path!");
                }
                arg_editor_path = args[i + 1];
                i += 1;
            }
            // Unexpected argument
            else {
                throw std::runtime_error("Invalid arguments!");
            }
        }
    } catch (std::runtime_error const& err) {
        // In case of error, reset settings
        arg_client_host.Clear();
        arg_client_port = 0;
        arg_server_port = 0;
        arg_editor_path.Clear();
        throw;
    }
}

void App::initHeadless()
{
    engineParameters_["Headless"] = true;
}

void App::initWindow()
{
    engineParameters_["FullScreen"] = false;
    engineParameters_["WindowWidth"] = 1280;
    engineParameters_["WindowHeight"] = 720;
    engineParameters_["WindowResizable"] = true;
}

}
