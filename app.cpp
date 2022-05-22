#include "app.hpp"

#include "gamestate.hpp"
#include "serverstate.hpp"
#include "editorstate.hpp"

#include "../urhoextras/mathutils.hpp"

#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/IO/Log.h>

#include <stdexcept>

namespace GameLib
{

App::App(Urho3D::Context* context) :
    UrhoExtras::States::StateManager(context),
    is_local(false),
    arg_server_port(0),
    arg_client_port(0),
    gamestate(NULL)
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
        is_local = true;
        pushState(Urho3D::SharedPtr<GameState>(new GameState(this, context_, arg_client_host, arg_client_port)));
    }
    // If scene editor
    else if (!arg_editor_path.Empty()) {
        pushState(Urho3D::SharedPtr<EditorState>(new EditorState(this, context_, arg_editor_path)));
    }
    // If no arguments are given, then connect to default server
    else {
        is_local = true;
        pushState(Urho3D::SharedPtr<GameState>(new GameState(this, context_, getDefaultHost(), getDefaultPort())));
    }
}

Urho3D::Scene* App::getScene()
{
    return scene;
}

bool App::isLocal() const
{
    return is_local;
}

void App::initializeSceneOnServer()
{
}

void App::initializeSceneOnClient()
{
}

void App::stepOnClient(float deltatime)
{
    (void)deltatime;
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

float App::getFogStartDistance() const
{
    return 1600;
}

float App::getFogEndDistance() const
{
    return 2000;
}

Urho3D::Color App::getFogColor() const
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

void App::addDecalToGameObjects(Urho3D::Material* mat, Urho3D::Vector3 const& pos, Urho3D::Vector3 const& dir, float size, float aspect, float depth, Urho3D::Vector2 const& uv_begin, Urho3D::Vector2 const& uv_end)
{
    // Convert direction to pitch and yaw
    float pitch;
    float yaw;
    UrhoExtras::getPitchAndYaw(pitch, yaw, dir);
    // Start with random roll
    Urho3D::Quaternion rot(Urho3D::Random(360.0f), Urho3D::Vector3::FORWARD);
    // Apply pitch and yaw
    rot = Urho3D::Quaternion(pitch, Urho3D::Vector3::RIGHT) * rot;
    rot = Urho3D::Quaternion(yaw, Urho3D::Vector3::UP) * rot;
    // Add decal
    addDecalToGameObjects(mat, pos, rot, size, aspect, depth, uv_begin, uv_end);
}

void App::addDecalToGameObjects(Urho3D::Material* mat, Urho3D::Vector3 const& pos, Urho3D::Quaternion const& rot, float size, float aspect, float depth, Urho3D::Vector2 const& uv_begin, Urho3D::Vector2 const& uv_end)
{
    // Create a frustum for decal. This can be used to discard drawables that we are not gonna hit.
    Urho3D::Vector3 abs_pos_adjusted = pos - 0.5f * depth * (rot * Urho3D::Vector3::FORWARD);
    Urho3D::Matrix3x4 frustum_transf = Urho3D::Matrix3x4(abs_pos_adjusted, rot, 1.0f);
    Urho3D::Frustum frustum;
    frustum.DefineOrtho(size, aspect, 1.0, 0.0f, depth, frustum_transf);

    // Iterate all GameObjects and try to add decal to them
    Urho3D::PODVector<Urho3D::Node*> children = scene->GetChildren(false);
    for (unsigned child_i = 0; child_i < children.Size(); ++ child_i) {
        Urho3D::Node* child_node = children[child_i];
        for (unsigned comp_i = 0; comp_i < child_node->GetNumComponents(); ++ comp_i) {
            Urho3D::Component* component = child_node->GetComponents()[comp_i];
            GameObject* gameobj = dynamic_cast<GameObject*>(component);
            if (gameobj && gameobj->receiveDecals()) {
                gamestate->addDecalsRecursively(child_node, frustum, mat, pos, rot, size, aspect, depth, uv_begin, uv_end);
            }
        }
    }
}

void App::setGameState(GameState* gamestate)
{
    this->gamestate = gamestate;
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
